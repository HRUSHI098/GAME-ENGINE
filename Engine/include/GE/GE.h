#pragma once

// ─── Core engine include — use this in game/app code ─────────────────────────

#include "GE/Core/Base.h"
#include "GE/Core/Log.h"
#include "GE/Core/Application.h"
#include "GE/Core/Layer.h"
#include "GE/Core/LayerStack.h"
#include "GE/Core/Input.h"
#include "GE/Core/Window.h"

// Events
#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/WindowEvents.h"
#include "GE/Core/Events/KeyEvents.h"
#include "GE/Core/Events/MouseEvents.h"

// Renderer
#include "GE/Renderer/Renderer2D.h"
#include "GE/Renderer/Camera.h"
#include "GE/Renderer/Texture.h"

// ECS
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"

// Physics
#include "GE/Physics/PhysicsComponents.h"
#include "GE/Physics/Physics2D.h"

// Audio
#include "GE/Audio/AudioEngine.h"
#include "GE/Audio/AudioClip.h"
#include "GE/Audio/AudioComponents.h"

// Scripting
#ifdef GE_SCRIPTING_ENABLED
#include "GE/Scripting/ScriptEngine.h"
#include "GE/Scripting/ScriptComponent.h"
#endif

// Editor
#include "GE/Editor/EditorLayer.h"

// AI / ML
#include "GE/AI/PythonEngine.h"
#include "GE/AI/AIComponent.h"
#include "GE/AI/BehaviorTree.h"
#include "GE/AI/MLEnvironment.h"
#include "GE/AI/AISystem.h"

// Math (re-export glm)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
