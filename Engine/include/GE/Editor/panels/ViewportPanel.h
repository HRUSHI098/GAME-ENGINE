#pragma once
#include <imgui.h>

namespace GE {

class ViewportPanel {
public:
    void OnImGuiRender();

    int  GetWidth()   const { return m_Width; }
    int  GetHeight()  const { return m_Height; }
    bool IsHovered()  const { return m_Hovered; }
    bool IsFocused()  const { return m_Focused; }

private:
    int  m_Width   = 1280;
    int  m_Height  = 720;
    bool m_Hovered = false;
    bool m_Focused = false;
};

} // namespace GE
