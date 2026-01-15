#pragma once

#include "engine.h"
#include "project.h"
#include "ui.h"

namespace unreal
{

class App
{
  public:
    App();
    ~App();

    bool init();
    void run();
    void shutdown();

  private:
    void loadConfig();
    void saveConfig();

    UI m_ui;
    ProjectManager m_projectManager;
    EngineManager m_engineManager;
};

} // namespace unreal
