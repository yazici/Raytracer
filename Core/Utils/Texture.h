#pragma once

#include "../RayLib.h"
#include "../Math/Vector4.h"

#include <memory>

namespace rt {

struct TextureEvaluator;

/**
 * Class representing 2D texture.
 */
class ITexture
{
public:
    RAYLIB_API virtual ~ITexture();

    // get human-readable description
    virtual const char* GetName() const = 0;

    // evaluate texture color at given coordinates
    virtual const math::Vector4 Evaluate(const math::Vector4& coords, const TextureEvaluator& evaluator) const = 0;
};

using TexturePtr = std::shared_ptr<ITexture>;

} // namespace rt
