#include "app.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    // Setup logging
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");

    spdlog::info("Starting Unreal Launcher v1.0.0");

    unreal::App app;

    if (!app.init())
    {
        spdlog::error("Failed to initialize application");
        return 1;
    }

    app.run();

    return 0;
}
