#pragma once

#include "GE/Renderer/Renderer2D.h"
#include <imgui.h>

namespace GE {

class StatsPanel {
public:
    void OnImGuiRender(f32 deltaTime) {
        ImGui::SetNextWindowPos(ImVec2(0, 420),   ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(220, 160), ImGuiCond_FirstUseEver);
        ImGui::Begin("Renderer Stats");

        // Frame timing
        m_FrameTimeAccum += deltaTime;
        ++m_FrameCount;
        if (m_FrameTimeAccum >= 0.25f) {   // update display 4x/s
            m_DisplayDt  = m_FrameTimeAccum / static_cast<f32>(m_FrameCount);
            m_DisplayFPS = 1.0f / m_DisplayDt;
            m_FrameTimeAccum = 0.0f;
            m_FrameCount     = 0;
        }

        ImGui::Text("Frame time : %.3f ms", m_DisplayDt * 1000.0f);
        ImGui::Text("FPS        : %.1f",    m_DisplayFPS);
        ImGui::Separator();

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Draw calls : %u", stats.DrawCalls);
        ImGui::Text("Quads      : %u", stats.QuadCount);

        ImGui::End();

        Renderer2D::ResetStats();
    }

private:
    f32 m_FrameTimeAccum = 0.0f;
    u32 m_FrameCount     = 0;
    f32 m_DisplayDt      = 0.0f;
    f32 m_DisplayFPS     = 0.0f;
};

} // namespace GE
