#pragma once

namespace GE {

class Scene;

// Called by Scene — declared here to keep Scene.h free of pybind11 headers
void AISystem_OnSceneStart(Scene* scene);
void AISystem_OnUpdate    (Scene* scene, float dt);
void AISystem_OnSceneStop (Scene* scene);

} // namespace GE
