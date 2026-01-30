#include "project.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <spdlog/spdlog.h>

namespace unreal
{

bool Project::isValid() const
{
    return !name.empty() && std::filesystem::exists(path) && std::filesystem::exists(uprojectPath);
}

std::string Project::getEngineVersionFromFile() const
{
    try
    {
        if (!std::filesystem::exists(uprojectPath))
        {
            return "";
        }

        std::ifstream file(uprojectPath);
        if (!file.is_open())
            return "";

        nlohmann::json json;
        file >> json;

        if (json.contains("EngineAssociation"))
        {
            return json["EngineAssociation"].get<std::string>();
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to read engine version from {}: {}", uprojectPath.string(), e.what());
    }
    return "";
}

void ProjectManager::log(const std::string& message, bool isError)
{
    if (m_logCallback)
    {
        m_logCallback(message, isError);
    }
    if (isError)
    {
        spdlog::error("{}", message);
    }
    else
    {
        spdlog::info("{}", message);
    }
}

std::optional<std::filesystem::path> ProjectManager::findUProjectFile(const std::filesystem::path& directory)
{
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".uproject")
            {
                return entry.path();
            }
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to scan directory {}: {}", directory.string(), e.what());
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> ProjectManager::findProjectIcon(const std::filesystem::path& uprojectPath)
{
    auto iconPath = uprojectPath;
    iconPath.replace_extension(".png");

    if (std::filesystem::exists(iconPath))
    {
        return iconPath;
    }

    // Also check in the project directory for an icon with the same name
    auto projectDir = uprojectPath.parent_path();
    auto projectName = uprojectPath.stem().string();
    auto altIconPath = projectDir / (projectName + ".png");

    if (std::filesystem::exists(altIconPath))
    {
        return altIconPath;
    }

    return std::nullopt;
}

bool ProjectManager::addProject(const std::filesystem::path& projectPath)
{
    auto uprojectFile = findUProjectFile(projectPath);
    if (!uprojectFile)
    {
        log("No .uproject file found in: " + projectPath.string(), true);
        return false;
    }

    // Check if already exists
    auto projectName = uprojectFile->stem().string();
    for (const auto& proj : m_projects)
    {
        if (proj.name == projectName && proj.uprojectPath == *uprojectFile)
        {
            log("Project already exists: " + projectName, true);
            return false;
        }
    }

    Project project;
    project.name = projectName;
    project.path = projectPath;
    project.uprojectPath = *uprojectFile;
    project.engineVersion = project.getEngineVersionFromFile();
    project.iconPath = findProjectIcon(*uprojectFile);

    m_projects.push_back(project);
    log("Added project: " + projectName);
    return true;
}

bool ProjectManager::addProjectsFromFolder(const std::filesystem::path& folderPath)
{
    if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
    {
        log("Invalid folder path: " + folderPath.string(), true);
        return false;
    }

    bool addedAny = false;
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath))
        {
            if (entry.is_directory())
            {
                auto uprojectFile = findUProjectFile(entry.path());
                if (uprojectFile)
                {
                    if (addProject(entry.path()))
                    {
                        addedAny = true;
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        log("Failed to scan folder: " + std::string(e.what()), true);
        return false;
    }

    return addedAny;
}

void ProjectManager::removeProject(const std::filesystem::path& uprojectPath)
{
    auto it = std::remove_if(m_projects.begin(), m_projects.end(),
        [&](const Project& p)
        {
            return p.uprojectPath == uprojectPath;
        });

    if (it != m_projects.end())
    {
        m_projects.erase(it, m_projects.end());
        log("Removed project: " + uprojectPath.string());
    }
}

Project* ProjectManager::findProject(const std::filesystem::path& uprojectPath)
{
    for (auto& proj : m_projects)
    {
        if (proj.uprojectPath == uprojectPath)
            return &proj;
    }
    return nullptr;
}

bool ProjectManager::load(const std::filesystem::path& configPath)
{
    try
    {
        if (!std::filesystem::exists(configPath))
        {
            log("Project config file does not exist: " + configPath.string());
            return false;
        }

        std::ifstream file(configPath);
        if (!file.is_open())
        {
            log("Failed to open project config: " + configPath.string(), true);
            return false;
        }

        nlohmann::json json;
        file >> json;

        m_projects.clear();
        for (const auto& item : json["projects"])
        {
            Project proj;
            proj.name = item["name"].get<std::string>();
            proj.path = item["path"].get<std::string>();
            proj.uprojectPath = item["uprojectPath"].get<std::string>();
            proj.engineVersion = item["engineVersion"].get<std::string>();

            if (item.contains("iconPath") && !item["iconPath"].is_null())
            {
                proj.iconPath = item["iconPath"].get<std::string>();
            }

            if (item.contains("commandLineArgs"))
            {
                proj.commandLineArgs = item["commandLineArgs"].get<std::string>();
            }

            // Verify the project still exists
            if (std::filesystem::exists(proj.uprojectPath))
            {
                m_projects.push_back(proj);
            }
            else
            {
                log("Project no longer exists, skipping: " + proj.name, true);
            }
        }

        log("Loaded " + std::to_string(m_projects.size()) + " projects from " + configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        log("Failed to load project config: " + std::string(e.what()), true);
        return false;
    }
}

bool ProjectManager::save(const std::filesystem::path& configPath) const
{
    try
    {
        // Create parent directories if needed
        auto parentPath = configPath.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath))
        {
            std::filesystem::create_directories(parentPath);
        }

        nlohmann::json json;
        json["projects"] = nlohmann::json::array();

        for (const auto& proj : m_projects)
        {
            nlohmann::json item;
            item["name"] = proj.name;
            item["path"] = proj.path.string();
            item["uprojectPath"] = proj.uprojectPath.string();
            item["engineVersion"] = proj.engineVersion;
            item["iconPath"] = proj.iconPath ? proj.iconPath->string() : "";
            item["commandLineArgs"] = proj.commandLineArgs;
            json["projects"].push_back(item);
        }

        std::ofstream file(configPath);
        if (!file.is_open())
        {
            spdlog::error("Failed to save project config: {}", configPath.string());
            return false;
        }

        file << json.dump(4);
        spdlog::info("Saved {} projects to {}", m_projects.size(), configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to save project config: {}", e.what());
        return false;
    }
}

} // namespace unreal
