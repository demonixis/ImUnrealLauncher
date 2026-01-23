#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace unreal
{

struct EngineVersion
{
    std::string versionName;
    std::string displayName;
    std::filesystem::path path;

    bool isValid() const;
    std::string getEditorPath() const;
    std::string getBuildToolPath() const;
    std::string getGenerateScriptPath() const;
};

class EngineManager
{
  public:
    EngineManager() = default;

    void addVersion(const std::string& name, const std::filesystem::path& path);
    void updateVersion(const std::string& oldName, const std::string& newName, const std::filesystem::path& newPath);
    void removeVersion(const std::string& name);
    const std::vector<EngineVersion>& getVersions() const
    {
        return m_versions;
    }
    const EngineVersion* findVersion(const std::string& name) const;

    bool load(const std::filesystem::path& configPath);
    bool save(const std::filesystem::path& configPath) const;

  private:
    std::vector<EngineVersion> m_versions;
};

} // namespace unreal
