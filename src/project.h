#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace unreal
{

struct Project
{
    std::string name;
    std::filesystem::path path;
    std::filesystem::path uprojectPath;
    std::string engineVersion;
    std::optional<std::filesystem::path> iconPath;
    std::string commandLineArgs;

    bool isValid() const;
    std::string getEngineVersionFromFile() const;
};

class ProjectManager
{
  public:
    using LogCallback = std::function<void(const std::string&, bool)>;

    ProjectManager() = default;

    void setLogCallback(LogCallback callback)
    {
        m_logCallback = callback;
    }

    bool addProject(const std::filesystem::path& path);
    bool addProjectsFromFolder(const std::filesystem::path& folderPath);
    void removeProject(const std::filesystem::path& folderPath);

    const std::vector<Project>& getProjects() const
    {
        return m_projects;
    }
    Project* findProject(const std::filesystem::path& uprojectPath);

    bool load(const std::filesystem::path& configPath);
    bool save(const std::filesystem::path& configPath) const;

    static std::optional<std::filesystem::path> findUProjectFile(const std::filesystem::path& directory);
    static std::optional<std::filesystem::path> findProjectIcon(const std::filesystem::path& uprojectPath);

  private:
    void log(const std::string& message, bool isError = false);

    std::vector<Project> m_projects;
    LogCallback m_logCallback;
};

} // namespace unreal
