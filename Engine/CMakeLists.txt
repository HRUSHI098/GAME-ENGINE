add_library(Engine STATIC)

target_sources(Engine PRIVATE
    # Core
    src/Core/Log.cpp
    src/Core/Application.cpp
    src/Core/LayerStack.cpp

    # Platform — SDL2
    src/Platform/SDL2/SDL2Window.cpp
    src/Platform/SDL2/SDL2Renderer.cpp
    src/Platform/SDL2/SDL2Input.cpp
    src/Platform/SDL2/SDL2Texture.cpp

    # Renderer
    src/Renderer/Camera.cpp
    src/Renderer/Renderer2D.cpp
    src/Renderer/SubTexture2D.cpp

    # ECS
    src/ECS/Scene.cpp

    # Physics
    src/Physics/Physics2D.cpp

    # Audio
    src/Platform/SDL2/SDL2Audio.cpp

    # AI / ML / Python
    src/AI/PythonEngine.cpp
    src/AI/EngineBindings.cpp
    src/AI/AISystem.cpp
    src/AI/ScriptWatcher.cpp

    # Editor
    src/Editor/ImGuiLayer.cpp
    src/Editor/EditorLayer.cpp
    src/Editor/panels/SceneHierarchyPanel.cpp
    src/Editor/panels/PropertiesPanel.cpp
    src/Editor/panels/ViewportPanel.cpp
    src/Editor/panels/AssetBrowserPanel.cpp
    src/Editor/panels/ConsolePanel.cpp

    # Serialization
    src/Scene/SceneSerializer.cpp

    # Asset management
    src/Assets/AssetManager.cpp

    # Tilemap
    src/Tilemap/Tilemap.cpp
)

# ── C# Scripting (only compiled when Mono is available) ──────────────────────
if(MONO_FOUND)
    target_sources(Engine PRIVATE
        src/Scripting/ScriptEngine.cpp
        src/Scripting/ScriptGlue.cpp
    )
    target_link_libraries(Engine PUBLIC Mono::Mono)
    target_compile_definitions(Engine PUBLIC GE_SCRIPTING_ENABLED)

    # Compute Mono install root from the found include dir (…/include/mono-2.0 → …)
    get_filename_component(_MONO_ROOT "${MONO_INCLUDE_DIR}/../.." ABSOLUTE)
    target_compile_definitions(Engine PRIVATE
        GE_MONO_ROOT="${_MONO_ROOT}")
    message(STATUS "[Engine] Mono scripting linked. Root: ${_MONO_ROOT}")
endif()

target_include_directories(Engine
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Engine
    PUBLIC
        spdlog::spdlog
        glm::glm
        EnTT::EnTT
        SDL2::SDL2
        SDL2::SDL2main
        box2d
        pybind11::embed
        imgui
        nlohmann_json::nlohmann_json
)

# SDL2_mixer — try both alias styles from FetchContent
foreach(_mix_target SDL2_mixer::SDL2_mixer SDL2_mixer)
    if(TARGET ${_mix_target})
        target_link_libraries(Engine PUBLIC ${_mix_target})
        get_target_property(_mix_inc ${_mix_target} INTERFACE_INCLUDE_DIRECTORIES)
        if(_mix_inc)
            target_include_directories(Engine PUBLIC ${_mix_inc})
        endif()
        break()
    endif()
endforeach()

# SDL2_image — try both alias styles from FetchContent
foreach(_img_target SDL2_image::SDL2_image SDL2_image SDL2::SDL2_image)
    if(TARGET ${_img_target})
        target_link_libraries(Engine PUBLIC ${_img_target})
        get_target_property(_img_inc ${_img_target} INTERFACE_INCLUDE_DIRECTORIES)
        if(_img_inc)
            target_include_directories(Engine PUBLIC ${_img_inc})
        endif()
        break()
    endif()
endforeach()

# Platform defines
if(WIN32)
    target_compile_definitions(Engine PUBLIC GE_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(Engine PUBLIC GE_PLATFORM_LINUX)
elseif(APPLE)
    target_compile_definitions(Engine PUBLIC GE_PLATFORM_MACOS)
endif()

# Build config defines
target_compile_definitions(Engine PUBLIC
    $<$<CONFIG:Debug>:GE_DEBUG>
    $<$<CONFIG:Release>:GE_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:GE_RELEASE>
)
