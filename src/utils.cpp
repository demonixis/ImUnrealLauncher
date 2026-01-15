#include "utils.h"
#include <array>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <climits>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

namespace unreal
{
void CommandExecutor::output(const std::string& message, bool isError)
{
    if (m_outputCallback)
    {
        m_outputCallback(message, isError);
    }
}

int CommandExecutor::execute(const std::string& command)
{
    m_running = true;
    m_cancelled = false;

    output("Executing: " + command);

#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        output("Failed to execute command", true);
        m_running = false;
        return -1;
    }

    std::array<char, 256> buffer;
    std::string line;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        if (m_cancelled)
            break;
        line = buffer.data();
        if (!line.empty() && line.back() == '\n')
            line.pop_back();
        if (!line.empty())
            output(line);
    }

    int result = _pclose(pipe);
#else
    // Use pipe + fork for real-time output on Unix
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        output("Failed to create pipe", true);
        m_running = false;
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        output("Failed to fork", true);
        close(pipefd[0]);
        close(pipefd[1]);
        m_running = false;
        return -1;
    }

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]); // Close read end

        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Disable buffering
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);

        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127); // exec failed
    }

    // Parent process
    close(pipefd[1]); // Close write end

    // Read output in real-time
    std::array<char, 256> buffer;
    std::string line;
    ssize_t bytesRead;

    while ((bytesRead = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0)
    {
        if (m_cancelled)
        {
            kill(pid, SIGTERM);
            break;
        }

        buffer[bytesRead] = '\0';
        std::string chunk(buffer.data());

        // Process line by line
        for (char c : chunk)
        {
            if (c == '\n')
            {
                if (!line.empty())
                {
                    output(line);
                    line.clear();
                }
            }
            else if (c != '\r')
            {
                line += c;
            }
        }
    }

    // Output any remaining content
    if (!line.empty())
    {
        output(line);
    }

    close(pipefd[0]);

    // Wait for child and get exit status
    int status;
    waitpid(pid, &status, 0);
    int result = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif

    m_running = false;

    if (result == 0)
    {
        output("[DONE] Command completed successfully");
    }
    else
    {
        output("[ERR] Command failed with code: " + std::to_string(result), true);
    }

    return result;
}

int CommandExecutor::execute(const std::vector<std::string>& args)
{
    if (args.empty())
        return -1;

    std::string command;
    for (const auto& arg : args)
    {
        if (!command.empty())
            command += " ";
        // Quote arguments with spaces
        if (arg.find(' ') != std::string::npos)
        {
            command += "\"" + arg + "\"";
        }
        else
        {
            command += arg;
        }
    }

    return execute(command);
}

std::future<int> CommandExecutor::executeAsync(const std::string& command)
{
    return std::async(std::launch::async, [this, command]() { return execute(command); });
}

std::future<int> CommandExecutor::executeAsync(const std::vector<std::string>& args)
{
    return std::async(std::launch::async, [this, args]() { return execute(args); });
}

void CommandExecutor::cancel()
{
    m_cancelled = true;
}

// ProjectOperations

ProjectOperations::ProjectOperations(LogCallback callback) : m_logCallback(callback)
{
    m_executor.setOutputCallback(callback);
}

std::future<bool> ProjectOperations::clean(const std::filesystem::path& projectPath)
{
    return std::async(std::launch::async,
                      [this, projectPath]()
                      {
                          m_logCallback("Cleaning project: " + projectPath.string(), false);

                          std::vector<std::string> foldersToDelete = {"Binaries", "DerivedDataCache", "Intermediate",
                                                                      "Saved", "Script"};

                          bool success = true;
                          for (const auto& folder : foldersToDelete)
                          {
                              auto folderPath = projectPath / folder;
                              if (std::filesystem::exists(folderPath))
                              {
                                  try
                                  {
                                      std::filesystem::remove_all(folderPath);
                                      m_logCallback("Deleted: " + folderPath.string(), false);
                                  }
                                  catch (const std::exception& e)
                                  {
                                      m_logCallback("Failed to delete " + folderPath.string() + ": " + e.what(), true);
                                      success = false;
                                  }
                              }
                          }

                          // Clean plugins
                          auto pluginsPath = projectPath / "Plugins";
                          if (std::filesystem::exists(pluginsPath))
                          {
                              try
                              {
                                  for (const auto& entry : std::filesystem::directory_iterator(pluginsPath))
                                  {
                                      if (entry.is_directory())
                                      {
                                          for (const auto& subfolder : {"Binaries", "Intermediate"})
                                          {
                                              auto subPath = entry.path() / subfolder;
                                              if (std::filesystem::exists(subPath))
                                              {
                                                  std::filesystem::remove_all(subPath);
                                                  m_logCallback("Deleted: " + subPath.string(), false);
                                              }
                                          }
                                      }
                                  }
                              }
                              catch (const std::exception& e)
                              {
                                  m_logCallback("Failed to clean plugins: " + std::string(e.what()), true);
                                  success = false;
                              }
                          }

                          if (success)
                          {
                              m_logCallback("[DONE] Project cleaned successfully", false);
                          }
                          return success;
                      });
}

std::future<bool> ProjectOperations::generateProjectFiles(const std::filesystem::path& enginePath,
                                                          const std::filesystem::path& uprojectPath)
{

    return std::async(std::launch::async,
                      [this, enginePath, uprojectPath]()
                      {
                          m_logCallback("Generating project files...", false);

#ifdef _WIN32
                          auto script = enginePath / "Engine" / "Build" / "BatchFiles" / "GenerateProjectFiles.bat";
#elif __APPLE__
                          auto script = enginePath / "Engine" / "Build" / "BatchFiles" / "Mac" / "GenerateProjectFiles.sh";
#else
                          auto script = enginePath / "Engine" / "Build" / "BatchFiles" / "Linux" / "GenerateProjectFiles.sh";
#endif

                          std::string command =
                              "\"" + script.string() + "\" \"" + uprojectPath.string() + "\" -game 2>&1";

                          int result = m_executor.execute(command);
                          return result == 0;
                      });
}

std::future<bool> ProjectOperations::build(const std::filesystem::path& enginePath,
                                           const std::filesystem::path& uprojectPath, BuildConfiguration config)
{

    return std::async(std::launch::async,
                      [this, enginePath, uprojectPath, config]()
                      {
                          auto projectName = uprojectPath.stem().string();
                          std::string configStr = buildConfigToString(config);
                          std::string target = projectName + "Editor";

#ifdef _WIN32
                          auto buildScript = enginePath / "Engine" / "Build" / "BatchFiles" / "Build.bat";
                          std::string platform = "Win64";
#elif __APPLE__
                          auto buildScript = enginePath / "Engine" / "Build" / "BatchFiles" / "Mac" / "Build.sh";
                          std::string platform = "Mac";
#else
                          auto buildScript = enginePath / "Engine" / "Build" / "BatchFiles" / "Linux" / "Build.sh";
                          std::string platform = "Linux";
#endif

                          m_logCallback("Building " + target + " (" + platform + " " + configStr + ")...", false);

                          std::string command = "\"" + buildScript.string() + "\" " + target + " " + platform + " " +
                                                configStr + " -Project=\"" + uprojectPath.string() +
                                                "\" -WaitMutex -Progress -NoHotReload 2>&1";

                          int result = m_executor.execute(command);
                          return result == 0;
                      });
}

std::future<bool> ProjectOperations::run(const std::filesystem::path& enginePath,
                                         const std::filesystem::path& uprojectPath, const std::string& additionalArgs)
{

    return std::async(std::launch::async,
                      [this, enginePath, uprojectPath, additionalArgs]()
                      {
                          m_logCallback("Launching project...", false);

#ifdef _WIN32
                          auto editor = enginePath / "Engine" / "Binaries" / "Win64" / "UnrealEditor.exe";
#elif __APPLE__
                          auto editor = enginePath / "Engine" / "Binaries" / "Mac" / "UnrealEditor.app" /
                                        "Contents" / "MacOS" / "UnrealEditor";
#else
                          auto editor = enginePath / "Engine" / "Binaries" / "Linux" / "UnrealEditor";
#endif

                          std::string command = "\"" + editor.string() + "\" \"" + uprojectPath.string() + "\"";
                          if (!additionalArgs.empty())
                          {
                              command += " " + additionalArgs;
                          }
                          command += " 2>&1";

                          int result = m_executor.execute(command);
                          return result == 0;
                      });
}

std::future<bool> ProjectOperations::package(const std::filesystem::path& enginePath,
                                             const std::filesystem::path& uprojectPath, Platform platform,
                                             const std::filesystem::path& outputPath)
{

    return std::async(std::launch::async,
                      [this, enginePath, uprojectPath, platform, outputPath]()
                      {
                          m_logCallback("Packaging project for " + platformToString(platform) + "...", false);

                          std::string platformStr;
                          switch (platform)
                          {
                              case Platform::Windows:
                                  platformStr = "Win64";
                                  break;
                              case Platform::Linux:
                                  platformStr = "Linux";
                                  break;
                              case Platform::Mac:
                                  platformStr = "Mac";
                                  break;
                              case Platform::Android:
                                  platformStr = "Android";
                                  break;
                          }

#ifdef _WIN32
                          auto uatPath = enginePath / "Engine" / "Build" / "BatchFiles" / "RunUAT.bat";
#else
        auto uatPath = enginePath / "Engine" / "Build" / "BatchFiles" / "RunUAT.sh";
#endif

                          std::string command = "\"" + uatPath.string() + "\" BuildCookRun " + "-project=\"" +
                                                uprojectPath.string() + "\" " + "-noP4 -platform=" + platformStr + " " +
                                                "-clientconfig=Shipping -serverconfig=Shipping " +
                                                "-cook -allmaps -build -stage -pak -archive " + "-archivedirectory=\"" +
                                                outputPath.string() + "\"";

                          int result = m_executor.execute(command);
                          return result == 0;
                      });
}

void ProjectOperations::cancel()
{
    m_executor.cancel();
}

// Utility functions

Platform getCurrentPlatform()
{
#ifdef _WIN32
    return Platform::Windows;
#elif __APPLE__
    return Platform::Mac;
#else
    return Platform::Linux;
#endif
}

std::string platformToString(Platform platform)
{
    switch (platform)
    {
        case Platform::Windows:
            return "Windows";
        case Platform::Linux:
            return "Linux";
        case Platform::Mac:
            return "Mac";
        case Platform::Android:
            return "Android";
    }
    return "Unknown";
}

std::string buildConfigToString(BuildConfiguration config)
{
    switch (config)
    {
        case BuildConfiguration::Development:
            return "Development";
        case BuildConfiguration::Shipping:
            return "Shipping";
        case BuildConfiguration::Debug:
            return "Debug";
    }
    return "Development";
}

std::filesystem::path getExecutablePath()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
#elif __APPLE__
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
    {
        return std::filesystem::canonical(buffer).parent_path();
    }
    return std::filesystem::current_path();
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).parent_path();
    }
    return std::filesystem::current_path();
#endif
}

std::filesystem::path getConfigDirectory()
{
    return getExecutablePath();
}

} // namespace unreal
