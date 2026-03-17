#pragma once

#include "GE/Core/Base.h"
#include <string>

namespace GE {

class Texture2D {
public:
    virtual ~Texture2D() = default;

    virtual u32 GetWidth()  const = 0;
    virtual u32 GetHeight() const = 0;

    virtual void* GetNativeHandle() const = 0;

    static Ref<Texture2D> Create(const std::string& path);
    static Ref<Texture2D> Create(u32 width, u32 height, void* data);
};

} // namespace GE
