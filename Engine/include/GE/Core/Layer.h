#pragma once

#include "GE/Core/Base.h"
#include "GE/Core/Events/Event.h"
#include <string>

namespace GE {

// Layers let you stack rendering/logic concerns (game world, UI, debug overlay).
class Layer {
public:
    explicit Layer(std::string name = "Layer") : m_DebugName(std::move(name)) {}
    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(f32 deltaTime) {}
    virtual void OnEvent(Event& event) {}
    virtual void OnImGuiRender() {}

    const std::string& GetName() const { return m_DebugName; }

protected:
    std::string m_DebugName;
};

} // namespace GE
