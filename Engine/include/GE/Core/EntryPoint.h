#pragma once

#include "GE/Core/Application.h"
#include "GE/Core/Log.h"
#include "GE/Renderer/Renderer2D.h"
#include "GE/Audio/AudioEngine.h"
#include "GE/AI/PythonEngine.h"
#ifdef GE_SCRIPTING_ENABLED
#include "GE/Scripting/ScriptEngine.h"
#endif

// Defined by the client (game project)
extern GE::Application* GE::CreateApplication();

int main(int argc, char** argv) {
    GE::Log::Init();
    GE_CORE_INFO("Engine v0.1 starting up.");

    auto* app = GE::CreateApplication();
    GE::PythonEngine::Init("scripts/ai");
#ifdef GE_SCRIPTING_ENABLED
    GE::ScriptEngine::Init("ScriptCore.dll");
    GE::ScriptEngine::LoadAppAssembly("SandboxScripts.dll");
#endif
    app->Run();
#ifdef GE_SCRIPTING_ENABLED
    GE::ScriptEngine::Shutdown();
#endif
    GE::PythonEngine::Shutdown();
    GE::AudioEngine::Shutdown();
    GE::Renderer2D::Shutdown();
    delete app;

    return 0;
}
