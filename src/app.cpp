#include "app.h"
#include "config.h"
#include <spdlog/spdlog.h>

namespace unreal
{

App::App() {}

App::~App()
{
    shutdown();
}

bool App::init()
{
    spdlog::info("Initializing Unreal Launcher...");

    // Setup log callback for project manager
    m_projectManager.setLogCallback([this](const std::string& msg, bool isError) { m_ui.log(msg, isError); });

    // Load configuration
    loadConfig();

    // Initialize UI
    if (!m_ui.init())
    {
        spdlog::error("Failed to initialize UI");
        return false;
    }

    // Connect managers to UI
    m_ui.setProjectManager(&m_projectManager);
    m_ui.setEngineManager(&m_engineManager);

    spdlog::info("Unreal Launcher initialized successfully");
    return true;
}

void App::run()
{
    while (!m_ui.shouldClose())
    {
        m_ui.render();
    }
}

void App::shutdown()
{
    spdlog::info("Shutting down Unreal Launcher...");

    // Save configuration
    saveConfig();

    m_ui.shutdown();

    spdlog::info("Unreal Launcher shutdown complete");
}

void App::loadConfig()
{
    auto& config = Config::instance();

    m_engineManager.load(config.getEnginesConfigPath());
    m_projectManager.load(config.getProjectsConfigPath());
}

void App::saveConfig()
{
    auto& config = Config::instance();

    m_engineManager.save(config.getEnginesConfigPath());
    m_projectManager.save(config.getProjectsConfigPath());
}

} // namespace unreal
