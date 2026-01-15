#pragma once

#include <filesystem>

namespace unreal
{

class Config
{
  public:
    static Config& instance();

    std::filesystem::path getEnginesConfigPath() const;
    std::filesystem::path getProjectsConfigPath() const;
    std::filesystem::path getAppConfigPath() const;
    std::filesystem::path getResourcesPath() const;

  private:
    Config();
    std::filesystem::path m_configDir;
};

} // namespace unreal
