#include "GE/Scripting/ScriptEngine.h"
#include "GE/Scripting/ScriptComponent.h"
#include "GE/ECS/Scene.h"
#include "GE/ECS/Entity.h"
#include "GE/ECS/Components.h"
#include "GE/Core/Log.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <fstream>
#include <vector>

namespace GE {

// ─── Static state ─────────────────────────────────────────────────────────────
MonoDomain*  ScriptEngine::s_RootDomain        = nullptr;
MonoDomain*  ScriptEngine::s_AppDomain         = nullptr;
MonoAssembly* ScriptEngine::s_CoreAssembly     = nullptr;
MonoImage*    ScriptEngine::s_CoreAssemblyImage = nullptr;
MonoAssembly* ScriptEngine::s_AppAssembly      = nullptr;
MonoImage*    ScriptEngine::s_AppAssemblyImage = nullptr;
Scene*        ScriptEngine::s_SceneContext     = nullptr;

std::unordered_map<std::string, ScriptClass>      ScriptEngine::s_EntityClasses;
std::unordered_map<u32, Scope<ScriptInstance>>    ScriptEngine::s_EntityInstances;

// ─── Utility: load a .dll from disk into a MonoAssembly ──────────────────────
static MonoAssembly* LoadMonoAssembly(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        GE_CORE_ERROR("[ScriptEngine] Cannot open assembly: {}", path);
        return nullptr;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> data(static_cast<size_t>(size));
    file.read(data.data(), size);

    MonoImageOpenStatus status;
    MonoImage* image = mono_image_open_from_data_full(
        data.data(), static_cast<u32>(size), true, &status, false);

    if (status != MONO_IMAGE_OK) {
        GE_CORE_ERROR("[ScriptEngine] mono_image_open_from_data error {}", status);
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, false);
    mono_image_close(image);
    return assembly;
}

// ─── ScriptEngine ─────────────────────────────────────────────────────────────
void ScriptEngine::Init(const std::string& coreAssemblyPath) {
    InitMono();
    RegisterInternalCalls();
    LoadCoreAssembly(coreAssemblyPath);
    GE_CORE_INFO("[ScriptEngine] Initialised. Core assembly: {}", coreAssemblyPath);
}

void ScriptEngine::Shutdown() {
    s_EntityInstances.clear();
    s_EntityClasses.clear();
    ShutdownMono();
    GE_CORE_INFO("[ScriptEngine] Shut down.");
}

void ScriptEngine::InitMono() {
    // Point Mono to its own standard library (mscorlib.dll etc.)
#ifdef GE_MONO_ROOT
    mono_set_dirs(GE_MONO_ROOT "/lib", GE_MONO_ROOT "/etc");
#endif

    s_RootDomain = mono_jit_init("GEJITRuntime");
    GE_ASSERT(s_RootDomain, "mono_jit_init failed");
}

void ScriptEngine::ShutdownMono() {
    mono_domain_set(s_RootDomain, false);
    if (s_AppDomain) {
        mono_domain_unload(s_AppDomain);
        s_AppDomain = nullptr;
    }
    mono_jit_cleanup(s_RootDomain);
    s_RootDomain = nullptr;
}

void ScriptEngine::LoadCoreAssembly(const std::string& path) {
    // Create an isolated app domain so we can unload + reload for hot-reload
    s_AppDomain = mono_domain_create_appdomain(
        const_cast<char*>("GEScriptRuntime"), nullptr);
    mono_domain_set(s_AppDomain, true);

    s_CoreAssembly      = LoadMonoAssembly(path);
    s_CoreAssemblyImage = mono_assembly_get_image(s_CoreAssembly);
}

bool ScriptEngine::LoadAppAssembly(const std::string& path) {
    s_AppAssembly      = LoadMonoAssembly(path);
    if (!s_AppAssembly) return false;
    s_AppAssemblyImage = mono_assembly_get_image(s_AppAssembly);

    // Enumerate all classes in the app assembly and register those that
    // inherit from GE.Entity (our script base class)
    s_EntityClasses.clear();

    MonoClass* entityBaseClass = mono_class_from_name(
        s_CoreAssemblyImage, "GE", "Entity");

    const MonoTableInfo* typeTable =
        mono_image_get_table_info(s_AppAssemblyImage, MONO_TABLE_TYPEDEF);
    int typeCount = mono_table_info_get_rows(typeTable);

    for (int i = 1; i < typeCount; ++i) {   // row 0 is the module pseudo-class
        u32 cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(s_AppAssemblyImage,
                                                          cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name      = mono_metadata_string_heap(s_AppAssemblyImage,
                                                          cols[MONO_TYPEDEF_NAME]);
        std::string fullName  = (nameSpace[0] != '\0')
                              ? std::string(nameSpace) + "." + name
                              : name;

        MonoClass* klass = mono_class_from_name(s_AppAssemblyImage, nameSpace, name);
        if (!klass || klass == entityBaseClass) continue;

        if (mono_class_is_subclass_of(klass, entityBaseClass, false)) {
            s_EntityClasses[fullName] = ScriptClass(nameSpace, name, s_AppAssemblyImage);
            GE_CORE_TRACE("[ScriptEngine] Registered script class '{}'.", fullName);
        }
    }

    GE_CORE_INFO("[ScriptEngine] App assembly loaded — {} script class(es).",
                 s_EntityClasses.size());
    return true;
}

bool ScriptEngine::ClassExists(const std::string& className) {
    return s_EntityClasses.count(className) != 0;
}

void ScriptEngine::OnSceneStart(Scene* scene) {
    s_SceneContext = scene;

    scene->View<ScriptComponent>([&](auto entity, ScriptComponent& sc) {
        if (!ClassExists(sc.ClassName)) {
            GE_CORE_WARN("[ScriptEngine] Unknown script class '{}' on entity {}.",
                         sc.ClassName, static_cast<u32>(entity));
            return;
        }
        Entity ent(entity, scene);
        auto instance = MakeScope<ScriptInstance>(s_EntityClasses.at(sc.ClassName), ent);
        instance->InvokeOnCreate();
        s_EntityInstances[static_cast<u32>(entity)] = std::move(instance);
    });
}

void ScriptEngine::OnSceneStop() {
    for (auto& [id, instance] : s_EntityInstances)
        instance->InvokeOnDestroy();
    s_EntityInstances.clear();
    s_SceneContext = nullptr;
}

void ScriptEngine::OnUpdate(Entity entity, f32 dt) {
    auto it = s_EntityInstances.find(static_cast<u32>(entity));
    if (it != s_EntityInstances.end())
        it->second->InvokeOnUpdate(dt);
}

// ─── ScriptClass ──────────────────────────────────────────────────────────────
ScriptClass::ScriptClass(const std::string& namespaceName, const std::string& className,
                         MonoImage* image)
    : m_Namespace(namespaceName), m_ClassName(className) {
    m_MonoClass = mono_class_from_name(image, namespaceName.c_str(), className.c_str());
    if (m_MonoClass) {
        m_OnCreate  = GetMethod("OnCreate",  0);
        m_OnUpdate  = GetMethod("OnUpdate",  1);
        m_OnDestroy = GetMethod("OnDestroy", 0);
    }
}

MonoObject* ScriptClass::Instantiate() const {
    MonoObject* obj = mono_object_new(ScriptEngine::GetAppDomain(), m_MonoClass);
    mono_runtime_object_init(obj);
    return obj;
}

void ScriptClass::InvokeOnCreate(MonoObject* instance) const {
    if (m_OnCreate) mono_runtime_invoke(m_OnCreate, instance, nullptr, nullptr);
}

void ScriptClass::InvokeOnUpdate(MonoObject* instance, f32 dt) const {
    if (m_OnUpdate) {
        void* args[] = { &dt };
        mono_runtime_invoke(m_OnUpdate, instance, args, nullptr);
    }
}

void ScriptClass::InvokeOnDestroy(MonoObject* instance) const {
    if (m_OnDestroy) mono_runtime_invoke(m_OnDestroy, instance, nullptr, nullptr);
}

MonoMethod* ScriptClass::GetMethod(const std::string& name, int paramCount) const {
    return mono_class_get_method_from_name(
        m_MonoClass, name.c_str(), paramCount);
}

// ─── ScriptInstance ───────────────────────────────────────────────────────────
ScriptInstance::ScriptInstance(const ScriptClass& scriptClass, Entity entity)
    : m_Class(scriptClass) {
    m_Instance = scriptClass.Instantiate();

    // Set the managed Entity.ID property before calling OnCreate
    MonoClass* klass = scriptClass.GetMonoClass();
    MonoClassField* idField = mono_class_get_field_from_name(
        mono_class_get_parent(klass), "<ID>k__BackingField");
    if (!idField) {
        // Try direct field name used by auto-properties
        idField = mono_class_get_field_from_name(
            mono_class_get_parent(klass), "ID");
    }
    if (idField) {
        u32 id = static_cast<u32>(entity);
        mono_field_set_value(m_Instance, idField, &id);
    }
}

void ScriptInstance::InvokeOnCreate()          { m_Class.InvokeOnCreate(m_Instance); }
void ScriptInstance::InvokeOnUpdate(f32 dt)    { m_Class.InvokeOnUpdate(m_Instance, dt); }
void ScriptInstance::InvokeOnDestroy()         { m_Class.InvokeOnDestroy(m_Instance); }

} // namespace GE
