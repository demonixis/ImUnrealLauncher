#include "ui.h"
#include "config.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>

namespace unreal
{

UI::UI() {}

UI::~UI()
{
    //shutdown();
}

bool UI::init()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_window = glfwCreateWindow(1024, 768, "Unreal Launcher", nullptr, nullptr);
    if (!m_window)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;

    // Initialize backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize operations
    m_operations =
        std::make_unique<ProjectOperations>([this](const std::string& msg, bool isError) { log(msg, isError); });

    // Load default icon
    auto defaultIconPath = Config::instance().getResourcesPath() / "default_icon.png";
    if (std::filesystem::exists(defaultIconPath))
    {
        int width, height, channels;
        unsigned char* data = stbi_load(defaultIconPath.string().c_str(), &width, &height, &channels, 4);
        if (data)
        {
            glGenTextures(1, &m_defaultIcon);
            glBindTexture(GL_TEXTURE_2D, m_defaultIcon);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }

    spdlog::info("UI initialized successfully");
    return true;
}

void UI::shutdown()
{
    // Clean up textures
    for (auto& [name, tex] : m_projectIcons)
    {
        if (tex)
            glDeleteTextures(1, &tex);
    }
    m_projectIcons.clear();

    if (m_defaultIcon)
    {
        glDeleteTextures(1, &m_defaultIcon);
        m_defaultIcon = 0;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool UI::shouldClose() const
{
    return m_window && glfwWindowShouldClose(m_window);
}

void UI::log(const std::string& message, bool isError)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logMessages.push_back({message, isError});
    if (m_logMessages.size() > MAX_LOG_LINES)
    {
        m_logMessages.pop_front();
    }
    m_logDirty = true;
}

void UI::loadProjectIcon(const Project& project)
{
    if (m_projectIcons.count(project.uprojectPath))
        return;

    GLuint texture = 0;
    if (project.iconPath && std::filesystem::exists(*project.iconPath))
    {
        int width, height, channels;
        unsigned char* data = stbi_load(project.iconPath->string().c_str(), &width, &height, &channels, 4);
        if (data)
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    m_projectIcons[project.uprojectPath] = texture;
}

GLuint UI::getProjectIcon(std::filesystem::path projectPath) const
{
    auto it = m_projectIcons.find(projectPath);
    if (it != m_projectIcons.end() && it->second != 0)
    {
        return it->second;
    }
    return m_defaultIcon;
}

void UI::render()
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Main menu bar
    renderMenuBar();

    // Main window covering the whole viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("MainWindow", nullptr, windowFlags);

    // Split into left (project list) and right (details)
    float listWidth = 300.0f;
    float logHeight = 150.0f;
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // Project list (left panel)
    ImGui::BeginChild("ProjectList", ImVec2(listWidth, contentSize.y - logHeight - 10), true);
    renderProjectList();
    ImGui::EndChild();

    ImGui::SameLine();

    // Project details (right panel)
    ImGui::BeginChild("ProjectDetails", ImVec2(0, contentSize.y - logHeight - 10), true);
    renderProjectDetails();
    ImGui::EndChild();

    // Log panel (bottom)
    ImGui::BeginChild("LogPanel", ImVec2(0, logHeight), true);
    renderLogPanel();
    ImGui::EndChild();

    ImGui::End();

    // Modal windows
    if (m_showEngineVersionsWindow)
    {
        renderEngineVersionsWindow();
    }
    if (m_showAddProjectWindow)
    {
        renderAddProjectWindow();
    }

    // Rendering
    ImGui::Render();
    int displayW, displayH;
    glfwGetFramebufferSize(m_window, &displayW, &displayH);
    glViewport(0, 0, displayW, displayH);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

void UI::renderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Add Project..."))
            {
                m_showAddProjectWindow = true;
                m_addProjectIsFolder = false;
                m_newProjectPath[0] = '\0';
            }
            if (ImGui::MenuItem("Add Projects from Folder..."))
            {
                m_showAddProjectWindow = true;
                m_addProjectIsFolder = true;
                m_newProjectPath[0] = '\0';
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Engine Versions..."))
            {
                m_showEngineVersionsWindow = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UI::renderProjectList()
{
    ImGui::Text("Projects");
    ImGui::Separator();

    if (!m_projectManager)
        return;

    const auto& projects = m_projectManager->getProjects();
    bool operationRunning = m_operations && m_operations->isRunning();

    for (const auto& project : projects)
    {
        loadProjectIcon(project);

        ImGui::PushID(project.uprojectPath.string().c_str());

        bool isSelected =
            m_selectedProject &&
            m_selectedProject->uprojectPath == project.uprojectPath;

        ImVec2 itemSize(ImGui::GetContentRegionAvail().x, 50.0f);

        ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;

        ImGui::BeginGroup();

        bool clicked = ImGui::Selectable(
            "##project",
            isSelected,
            flags,
            itemSize
        );
        
        // Double-click to launch (only if not running)
        if (!operationRunning && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            if (m_engineManager && m_selectedProject)
            {
                auto* engine = m_engineManager->findVersion(m_selectedProject->engineVersion);
                if (engine)
                {
                    std::thread([this, engine, project = m_selectedProject]()
                    {
                        m_operations->run(engine->path, project->uprojectPath, project->commandLineArgs);
                    }).detach();
                }
            }
        }
        
        ImGui::SameLine(10);

        GLuint icon = getProjectIcon(project.uprojectPath);
        ImGui::Image(
            (ImTextureID)icon,
            ImVec2(40, 40)
        );

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Text("%s", project.name.c_str());
        ImGui::TextColored(
            ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "UE %s",
            project.engineVersion.c_str()
        );
        ImGui::EndGroup();

        ImGui::EndGroup();

        if (clicked)
        {
            m_selectedProject =
                m_projectManager->findProject(project.uprojectPath);

            if (m_engineManager && m_selectedProject)
            {
                const auto& engines = m_engineManager->getVersions();
                for (size_t j = 0; j < engines.size(); ++j)
                {
                    if (engines[j].versionName == m_selectedProject->engineVersion)
                    {
                        m_selectedEngineIndex = (int)j;
                        break;
                    }
                }
            }

            strncpy(
                m_commandLineArgs,
                m_selectedProject->commandLineArgs.c_str(),
                sizeof(m_commandLineArgs) - 1
            );
            m_commandLineArgs[sizeof(m_commandLineArgs) - 1] = '\0';
        }

        ImGui::PopID();
    }
}

void UI::renderProjectDetails()
{
    ImGui::Text("Details");
    ImGui::Separator();

    if (!m_selectedProject)
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a project from the list");
        return;
    }

    // Project name and path
    ImGui::Text("Name: %s", m_selectedProject->name.c_str());
    ImGui::Text("Path: %s", m_selectedProject->path.string().c_str());
    ImGui::Spacing();

    // Engine version combo
    if (m_engineManager)
    {
        const auto& engines = m_engineManager->getVersions();
        if (!engines.empty())
        {
            ImGui::Text("Engine Version:");
            ImGui::SameLine();

            std::vector<const char*> engineNames;
            for (const auto& e : engines)
            {
                engineNames.push_back(e.versionName.c_str());
            }

            if (ImGui::Combo("##EngineVersion", &m_selectedEngineIndex, engineNames.data(),
                             static_cast<int>(engineNames.size())))
            {
                if (m_selectedEngineIndex >= 0 && m_selectedEngineIndex < static_cast<int>(engines.size()))
                {
                    m_selectedProject->engineVersion = engines[m_selectedEngineIndex].versionName;
                }
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No engine versions configured");
            if (ImGui::Button("Configure Engines"))
            {
                m_showEngineVersionsWindow = true;
            }
        }
    }

    ImGui::Spacing();

    // Command line arguments
    ImGui::Text("Command Line Arguments:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##CommandLineArgs", m_commandLineArgs, sizeof(m_commandLineArgs)))
    {
        m_selectedProject->commandLineArgs = m_commandLineArgs;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Action buttons
    bool operationRunning = m_operations && m_operations->isRunning();

    ImGui::BeginDisabled(operationRunning);

    if (ImGui::Button("Clean", ImVec2(100, 30)))
    {
        log("Starting clean operation...");
        m_currentOperation = m_operations->clean(m_selectedProject->path);
    }

    ImGui::SameLine();

    if (ImGui::Button("Generate", ImVec2(100, 30)))
    {
        if (m_engineManager)
        {
            auto* engine = m_engineManager->findVersion(m_selectedProject->engineVersion);
            if (engine)
            {
                log("Generating project files...");
                m_currentOperation = m_operations->generateProjectFiles(engine->path, m_selectedProject->uprojectPath);
            }
            else
            {
                log("Engine version not found: " + m_selectedProject->engineVersion, true);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Build", ImVec2(100, 30)))
    {
        if (m_engineManager)
        {
            auto* engine = m_engineManager->findVersion(m_selectedProject->engineVersion);
            if (engine)
            {
                log("Building project...");
                m_currentOperation = m_operations->build(engine->path, m_selectedProject->uprojectPath);
            }
            else
            {
                log("Engine version not found: " + m_selectedProject->engineVersion, true);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Run", ImVec2(100, 30)))
    {
        if (m_engineManager)
        {
            auto* engine = m_engineManager->findVersion(m_selectedProject->engineVersion);
            if (engine)
            {
                m_currentOperation = m_operations->run(engine->path, m_selectedProject->uprojectPath,
                                                       m_selectedProject->commandLineArgs);
            }
            else
            {
                log("Engine version not found: " + m_selectedProject->engineVersion, true);
            }
        }
    }

    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Package section
    ImGui::Text("Package:");

    static const char* platforms[] = {"Windows", "Linux", "Mac", "Android"};

    ImGui::SetNextItemWidth(150);
    ImGui::Combo("##Platform", &m_selectedPlatformIndex, platforms, IM_ARRAYSIZE(platforms));

    ImGui::SameLine();

    ImGui::BeginDisabled(operationRunning);
    if (ImGui::Button("Package", ImVec2(100, 0)))
    {
        if (m_engineManager)
        {
            auto* engine = m_engineManager->findVersion(m_selectedProject->engineVersion);
            if (engine)
            {
                Platform platform = static_cast<Platform>(m_selectedPlatformIndex);
                auto outputPath = m_selectedProject->path / "Package" / platformToString(platform);
                log("Packaging for " + platformToString(platform) + "...");
                m_currentOperation =
                    m_operations->package(engine->path, m_selectedProject->uprojectPath, platform, outputPath);
            }
            else
            {
                log("Engine version not found: " + m_selectedProject->engineVersion, true);
            }
        }
    }
    ImGui::EndDisabled();

    ImGui::Spacing();

    // Remove project button
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Remove Project", ImVec2(150, 0)))
    {
        ImGui::OpenPopup("Confirm Remove");
    }

    if (ImGui::BeginPopupModal("Confirm Remove", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Remove project '%s' from the list?", m_selectedProject->name.c_str());
        ImGui::Text("(This will not delete any files)");
        ImGui::Spacing();

        if (ImGui::Button("Yes", ImVec2(80, 0)))
        {
            auto pathToRemove  = m_selectedProject->uprojectPath;
            m_selectedProject = nullptr;
            m_projectManager->removeProject(pathToRemove);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(80, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UI::renderEngineVersionsWindow()
{
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Engine Versions", &m_showEngineVersionsWindow))
    {
        // Add new engine version
        ImGui::Text("Add Engine Version:");

        ImGui::InputText("Name", m_newEngineName, sizeof(m_newEngineName));
        ImGui::InputText("Path", m_newEnginePath, sizeof(m_newEnginePath));

        if (ImGui::Button("Add Engine"))
        {
            if (strlen(m_newEngineName) > 0 && strlen(m_newEnginePath) > 0)
            {
                m_engineManager->addVersion(m_newEngineName, m_newEnginePath);
                m_newEngineName[0] = '\0';
                m_newEnginePath[0] = '\0';
            }
        }

        ImGui::Separator();

        // List existing engines
        ImGui::Text("Configured Engines:");

        if (m_engineManager)
        {
            const auto& engines = m_engineManager->getVersions();

            if (ImGui::BeginTable("EnginesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100);
                ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 130);
                ImGui::TableHeadersRow();

                std::string toRemove;
                std::string toEdit;
                for (const auto& engine : engines)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", engine.versionName.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", engine.path.string().c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID(engine.versionName.c_str());
                    if (ImGui::Button("Edit"))
                    {
                        toEdit = engine.versionName;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Remove"))
                    {
                        toRemove = engine.versionName;
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();

                if (!toEdit.empty())
                {
                    auto* engine = m_engineManager->findVersion(toEdit);
                    if (engine)
                    {
                        m_editingEngine = true;
                        m_editingEngineName = engine->versionName;
                        strncpy(m_editEngineName, engine->versionName.c_str(), sizeof(m_editEngineName) - 1);
                        m_editEngineName[sizeof(m_editEngineName) - 1] = '\0';
                        strncpy(m_editEnginePath, engine->path.string().c_str(), sizeof(m_editEnginePath) - 1);
                        m_editEnginePath[sizeof(m_editEnginePath) - 1] = '\0';
                        ImGui::OpenPopup("Edit Engine");
                    }
                }

                if (!toRemove.empty())
                {
                    m_engineManager->removeVersion(toRemove);
                }
            }
        }

        // Edit engine popup
        if (ImGui::BeginPopupModal("Edit Engine", &m_editingEngine, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Edit Engine Version:");
            ImGui::Spacing();

            ImGui::InputText("Name##Edit", m_editEngineName, sizeof(m_editEngineName));
            ImGui::InputText("Path##Edit", m_editEnginePath, sizeof(m_editEnginePath));

            ImGui::Spacing();

            if (ImGui::Button("Save", ImVec2(100, 0)))
            {
                if (strlen(m_editEngineName) > 0 && strlen(m_editEnginePath) > 0)
                {
                    m_engineManager->updateVersion(m_editingEngineName, m_editEngineName, m_editEnginePath);
                    m_editingEngine = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0)))
            {
                m_editingEngine = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void UI::renderAddProjectWindow()
{
    ImGui::SetNextWindowSize(ImVec2(500, 150), ImGuiCond_FirstUseEver);

    const char* title = m_addProjectIsFolder ? "Add Projects from Folder" : "Add Project";

    if (ImGui::Begin(title, &m_showAddProjectWindow))
    {
        ImGui::Text(m_addProjectIsFolder ? "Folder Path:" : "Project Path:");
        ImGui::InputText("##Path", m_newProjectPath, sizeof(m_newProjectPath));

        if (ImGui::Button("Add"))
        {
            if (strlen(m_newProjectPath) > 0)
            {
                bool success;
                if (m_addProjectIsFolder)
                {
                    success = m_projectManager->addProjectsFromFolder(m_newProjectPath);
                }
                else
                {
                    success = m_projectManager->addProject(m_newProjectPath);
                }

                if (success)
                {
                    m_showAddProjectWindow = false;
                    m_newProjectPath[0] = '\0';
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            m_showAddProjectWindow = false;
            m_newProjectPath[0] = '\0';
        }
    }
    ImGui::End();
}

void UI::renderLogPanel()
{
    ImGui::Text("Log");
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logMessages.clear();
        m_logBuffer.clear();
        m_logDirty = false;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_logAutoScroll);
    ImGui::Separator();

    // Update log buffer only when dirty
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        if (m_logDirty)
        {
            m_logBuffer.clear();
            for (const auto& [msg, isError] : m_logMessages)
            {
                m_logBuffer += msg + "\n";
            }
            m_logDirty = false;
        }
    }

    // Use a child window with scrolling for better performance
    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Display each line individually for coloring
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        for (const auto& [msg, isError] : m_logMessages)
        {
            if (isError)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                ImGui::TextWrapped("%s", msg.c_str());
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::TextWrapped("%s", msg.c_str());
            }

            // Context menu for copying individual line
            if (ImGui::BeginPopupContextItem(""))
            {
                if (ImGui::Selectable("Copy line"))
                {
                    ImGui::SetClipboardText(msg.c_str());
                }
                if (ImGui::Selectable("Copy all"))
                {
                    ImGui::SetClipboardText(m_logBuffer.c_str());
                }
                ImGui::EndPopup();
            }
        }
    }

    // Auto-scroll to bottom
    if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10)
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

} // namespace unreal
