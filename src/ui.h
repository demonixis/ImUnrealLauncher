#pragma once

#include "engine.h"
#include "project.h"
#include "utils.h"
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

struct GLFWwindow;
typedef unsigned int GLuint;

namespace unreal
{

class UI
{
  public:
    UI();
    ~UI();

    bool init();
    void shutdown();
    void render();
    bool shouldClose() const;

    void setProjectManager(ProjectManager* pm)
    {
        m_projectManager = pm;
    }
    void setEngineManager(EngineManager* em)
    {
        m_engineManager = em;
    }

    void log(const std::string& message, bool isError = false);

  private:
    void renderMenuBar();
    void renderProjectList();
    void renderProjectDetails();
    void renderEngineVersionsWindow();
    void renderAddProjectWindow();
    void renderLogPanel();

    void loadProjectIcon(const Project& project);
    GLuint getProjectIcon(std::filesystem::path projectPath) const;

    GLFWwindow* m_window = nullptr;
    ProjectManager* m_projectManager = nullptr;
    EngineManager* m_engineManager = nullptr;

    // UI State
    Project* m_selectedProject = nullptr;
    int m_selectedEngineIndex = 0;
    int m_selectedPlatformIndex = 0;
    bool m_showEngineVersionsWindow = false;
    bool m_showAddProjectWindow = false;
    bool m_addProjectIsFolder = false;
    char m_newEngineName[256] = "";
    char m_newEngineAssociation[256] = "";
    char m_newEnginePath[1024] = "";
    char m_newProjectPath[1024] = "";

    // Engine editing state
    bool m_editingEngine = false;
    std::string m_editingEngineVersion;
    char m_editEngineName[256] = "";
    char m_editEngineAssociation[256] = "";
    char m_editEnginePath[1024] = "";

    // Project command line args
    char m_commandLineArgs[1024] = "";

    // Log
    std::deque<std::pair<std::string, bool>> m_logMessages;
    std::mutex m_logMutex;
    static constexpr size_t MAX_LOG_LINES = 500;
    std::string m_logBuffer;
    bool m_logDirty = false;
    bool m_logAutoScroll = true;

    // Icons
    std::unordered_map<std::filesystem::path, GLuint> m_projectIcons;
    GLuint m_defaultIcon = 0;

    // Operations
    std::unique_ptr<ProjectOperations> m_operations;
    std::future<bool> m_currentOperation;
};

} // namespace unreal
