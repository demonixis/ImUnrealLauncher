#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <vector>

namespace unreal
{
enum class Platform
{
    Windows,
    Linux,
    Mac,
    Android
};

enum class BuildConfiguration
{
    Development,
    Shipping,
    Debug
};

class CommandExecutor
{
  public:
    using OutputCallback = std::function<void(const std::string&, bool)>;

    CommandExecutor() = default;

    void setOutputCallback(OutputCallback callback)
    {
        m_outputCallback = callback;
    }

    // Synchronous execution
    int execute(const std::string& command);
    int execute(const std::vector<std::string>& args);

    // Asynchronous execution
    std::future<int> executeAsync(const std::string& command);
    std::future<int> executeAsync(const std::vector<std::string>& args);

    // Cancel running command
    void cancel();
    bool isRunning() const
    {
        return m_running;
    }

  private:
    void output(const std::string& message, bool isError = false);

    OutputCallback m_outputCallback;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_cancelled{false};
};

// Project operations
class ProjectOperations
{
  public:
    using LogCallback = std::function<void(const std::string&, bool)>;

    ProjectOperations(LogCallback callback);

    std::future<bool> clean(const std::filesystem::path& projectPath);
    std::future<bool> generateProjectFiles(const std::filesystem::path& enginePath,
                                           const std::filesystem::path& uprojectPath);
    std::future<bool> build(const std::filesystem::path& enginePath, const std::filesystem::path& uprojectPath,
                            BuildConfiguration config = BuildConfiguration::Development);
    std::future<bool> run(const std::filesystem::path& enginePath, const std::filesystem::path& uprojectPath,
                          const std::string& additionalArgs = "");
    std::future<bool> package(const std::filesystem::path& enginePath, const std::filesystem::path& uprojectPath,
                              Platform platform, const std::filesystem::path& outputPath);

    void cancel();
    bool isRunning() const
    {
        return m_executor.isRunning();
    }

  private:
    CommandExecutor m_executor;
    LogCallback m_logCallback;
};

// Utility functions
Platform getCurrentPlatform();
std::string platformToString(Platform platform);
std::string buildConfigToString(BuildConfiguration config);
std::filesystem::path getExecutablePath();
std::filesystem::path getConfigDirectory();

} // namespace unreal
