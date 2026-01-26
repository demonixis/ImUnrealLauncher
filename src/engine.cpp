#include "engine.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace unreal
{

bool EngineVersion::isValid() const
{
    if (engineAssociation.empty() || path.empty())
        return false;

    // Check if engine path exists and contains expected structure
    auto editorPath = std::filesystem::path(path) / "Engine" / "Binaries";
    return std::filesystem::exists(editorPath);
}

std::string EngineVersion::getEditorPath() const
{
#ifdef _WIN32
    return (path / "Engine" / "Binaries" / "Win64" / "UnrealEditor.exe").string();
#elif __APPLE__
    return (path / "Engine" / "Binaries" / "Mac" / "UnrealEditor.app" / "Contents" / "MacOS" / "UnrealEditor").string();
#else
    return (path / "Engine" / "Binaries" / "Linux" / "UnrealEditor").string();
#endif
}

std::string EngineVersion::getBuildToolPath() const
{
#ifdef _WIN32
    return (path / "Engine" / "Binaries" / "DotNET" / "UnrealBuildTool" / "UnrealBuildTool.dll").string();
#else
    return (path / "Engine" / "Build" / "BatchFiles" / "Linux" / "Build.sh").string();
#endif
}

std::string EngineVersion::getGenerateScriptPath() const
{
#ifdef _WIN32
    return (path / "Engine" / "Build" / "BatchFiles" / "GenerateProjectFiles.bat").string();
#else
    return (path / "GenerateProjectFiles.sh").string();
#endif
}

void EngineManager::addVersion(const std::string& association, const std::string& displayName, const std::filesystem::path& path)
{
    // Check if already exists
    for (auto& ver : m_versions)
    {
        if (ver.engineAssociation == association)
        {
            ver.displayName = displayName;
            ver.path = path;
            spdlog::info("Updated engine {} version: {} -> {}", displayName, association, path.string());
            return;
        }
    }

    EngineVersion version;
    version.engineAssociation = association;
    version.displayName = displayName;
    version.path = path;

    if (version.isValid())
    {
        m_versions.push_back(version);
        spdlog::info("Added engine version: {} at {}", displayName, path.string());
    }
    else
    {
        spdlog::warn("Invalid engine path: {}", path.string());
    }
}

void EngineManager::updateVersion(const std::string& association, const std::string& newDisplayName, const std::filesystem::path& newPath)
{
    for (auto& ver : m_versions)
    {
        if (ver.engineAssociation == association)
        {
            EngineVersion updated;
            updated.engineAssociation = association;
            updated.path = newPath;

            if (updated.isValid())
            {
                ver.displayName = newDisplayName;
                ver.path = newPath;
            }
            else
            {
                spdlog::warn("Invalid engine path for update: {}", newPath.string());
            }
            return;
        }
    }
    spdlog::warn("Engine version not found for update: {}", association);
}

void EngineManager::removeVersion(const std::string& association)
{
    auto it = std::remove_if(m_versions.begin(), m_versions.end(),
                             [&association](const EngineVersion& v) { return v.engineAssociation == association; });
    if (it != m_versions.end())
    {
        m_versions.erase(it, m_versions.end());
        spdlog::info("Removed engine version: {}", association);
    }
}

const EngineVersion* EngineManager::findByAssociation(
    const std::string& association) const
{
    for (const auto& ver : m_versions)
        if (ver.engineAssociation == association)
            return &ver;
    return nullptr;
}


bool EngineManager::load(const std::filesystem::path& configPath)
{
    try
    {
        if (!std::filesystem::exists(configPath))
        {
            spdlog::info("Engine config file does not exist: {}", configPath.string());
            return false;
        }

        std::ifstream file(configPath);
        if (!file.is_open())
        {
            spdlog::error("Failed to open engine config: {}", configPath.string());
            return false;
        }

        nlohmann::json json;
        file >> json;

        m_versions.clear();
        for (const auto& item : json["engines"])
        {
            EngineVersion ver;
            ver.engineAssociation = item["association"].get<std::string>();
            ver.displayName = item["displayName"].get<std::string>();
            ver.path = item["path"].get<std::string>();
            m_versions.push_back(ver);
        }

        spdlog::info("Loaded {} engine versions from {}", m_versions.size(), configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to load engine config: {}", e.what());
        return false;
    }
}

bool EngineManager::save(const std::filesystem::path& configPath) const
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
        json["engines"] = nlohmann::json::array();

        for (const auto& ver : m_versions)
        {
            nlohmann::json item;
            item["association"] = ver.engineAssociation;
            item["displayName"] = ver.displayName;
            item["path"] = ver.path.string();
            json["engines"].push_back(item);
        }

        std::ofstream file(configPath);
        if (!file.is_open())
        {
            spdlog::error("Failed to save engine config: {}", configPath.string());
            return false;
        }

        file << json.dump(4);
        spdlog::info("Saved {} engine versions to {}", m_versions.size(), configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to save engine config: {}", e.what());
        return false;
    }
}

} // namespace unreal
