#pragma once
#include "GE/ECS/Scene.h"
#include <string>

namespace GE {

class SceneSerializer {
public:
    explicit SceneSerializer(Scene* scene) : m_Scene(scene) {}

    void Serialize(const std::string& filepath);
    bool Deserialize(const std::string& filepath);

private:
    Scene* m_Scene;
};

} // namespace GE
