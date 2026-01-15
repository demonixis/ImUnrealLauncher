#include "config.h"
#include "utils.h"

namespace unreal
{

Config& Config::instance()
{
    static Config instance;
    return instance;
}

Config::Config()
{
    m_configDir = getExecutablePath();
}

std::filesystem::path Config::getEnginesConfigPath() const
{
    return m_configDir / "engines.json";
}

std::filesystem::path Config::getProjectsConfigPath() const
{
    return m_configDir / "projects.json";
}

std::filesystem::path Config::getAppConfigPath() const
{
    return m_configDir / "config.json";
}

std::filesystem::path Config::getResourcesPath() const
{
    return m_configDir / "resources";
}

} // namespace unreal
