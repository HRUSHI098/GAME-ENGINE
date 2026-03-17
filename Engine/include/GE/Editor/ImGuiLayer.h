#pragma once

#include "GE/Core/Layer.h"

namespace GE {

// Manages the ImGui context, SDL2 backend init/shutdown, and the per-frame
// Begin/End calls that wrap all ImGui draw commands.
class ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(Event& e) override;

    // Call Begin() before any ImGui:: calls, End() after the last one.
    void Begin();
    void End();

    void SetBlockEvents(bool block) { m_BlockEvents = block; }

private:
    bool m_BlockEvents = false;
};

} // namespace GE
