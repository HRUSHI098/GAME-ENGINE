#pragma once

#include "GE/Core/Layer.h"
#include <vector>

namespace GE {

// Layers pushed via PushLayer go below the overlay split point.
// Layers pushed via PushOverlay go above — they receive events first.
class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    std::vector<Layer*>::iterator       begin()       { return m_Layers.begin(); }
    std::vector<Layer*>::iterator       end()         { return m_Layers.end(); }
    std::vector<Layer*>::reverse_iterator rbegin()    { return m_Layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend()      { return m_Layers.rend(); }

private:
    std::vector<Layer*> m_Layers;
    u32 m_LayerInsertIndex = 0;
};

} // namespace GE
