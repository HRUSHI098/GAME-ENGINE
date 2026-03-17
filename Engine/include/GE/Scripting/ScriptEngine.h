#pragma once

#include "GE/Core/Base.h"
#include <string>
#include <unordered_map>
#include <functional>

// Forward-declare Mono types — don't leak Mono headers into game code
typedef struct _MonoDomain  MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage   MonoImage;
typedef struct _MonoClass   MonoClass;
typedef struct _MonoObject  MonoObject;
typedef struct _MonoMethod  MonoMethod;

namespace GE {

class Scene;
class Entity;

// ─── ScriptClass ──────────────────────────────────────────────────────────────
// Wraps a MonoClass and caches the lifecycle method pointers.
class ScriptClass {
public:
    ScriptClass() = default;
    ScriptClass(const std::string& namespaceName, const std::string& className,
                MonoImage* image);

    MonoObject* Instantiate() const;
    void        InvokeOnCreate (MonoObject* instance)              const;
    void        InvokeOnUpdate (MonoObject* instance, f32 dt)      const;
    void        InvokeOnDestroy(MonoObject* instance)              const;

    MonoClass* GetMonoClass() const { return m_MonoClass; }
    bool       IsValid()      const { return m_MonoClass != nullptr; }

private:
    MonoMethod* GetMethod(const std::string& name, int paramCount) const;

    std::string  m_Namespace;
    std::string  m_ClassName;
    MonoClass*   m_MonoClass  = nullptr;
    MonoMethod*  m_OnCreate   = nullptr;
    MonoMethod*  m_OnUpdate   = nullptr;
    MonoMethod*  m_OnDestroy  = nullptr;
};

// ─── ScriptInstance ───────────────────────────────────────────────────────────
// A live instance of a C# script attached to one entity.
class ScriptInstance {
public:
    ScriptInstance(const ScriptClass& scriptClass, Entity entity);
    ~ScriptInstance() = default;

    void InvokeOnCreate();
    void InvokeOnUpdate(f32 dt);
    void InvokeOnDestroy();

    MonoObject* GetManagedObject() const { return m_Instance; }

private:
    const ScriptClass& m_Class;
    MonoObject*        m_Instance = nullptr;
};

// ─── ScriptEngine ─────────────────────────────────────────────────────────────
class ScriptEngine {
public:
    static void Init(const std::string& coreAssemblyPath);
    static void Shutdown();

    // Load (or reload) the game script assembly at runtime
    static bool LoadAppAssembly(const std::string& assemblyPath);

    // Scene lifecycle — creates/destroys ScriptInstances
    static void OnSceneStart(Scene* scene);
    static void OnSceneStop();

    // Per-frame — called by Scene::OnUpdate for all ScriptComponent entities
    static void OnUpdate(Entity entity, f32 dt);

    // Check whether a C# class named `className` exists in the app assembly
    static bool ClassExists(const std::string& className);

    static Scene*       GetSceneContext() { return s_SceneContext; }
    static MonoDomain*  GetAppDomain()    { return s_AppDomain; }
    static MonoImage*   GetCoreImage()    { return s_CoreAssemblyImage; }
    static MonoImage*   GetAppImage()     { return s_AppAssemblyImage; }

private:
    static void InitMono();
    static void ShutdownMono();
    static void RegisterInternalCalls();
    static void LoadCoreAssembly(const std::string& path);

    static MonoDomain*  s_RootDomain;
    static MonoDomain*  s_AppDomain;

    static MonoAssembly* s_CoreAssembly;
    static MonoImage*    s_CoreAssemblyImage;
    static MonoAssembly* s_AppAssembly;
    static MonoImage*    s_AppAssemblyImage;

    static std::unordered_map<std::string, ScriptClass> s_EntityClasses;

    // entity ID → live script instance
    static std::unordered_map<u32, Scope<ScriptInstance>> s_EntityInstances;

    static Scene* s_SceneContext;
};

} // namespace GE
