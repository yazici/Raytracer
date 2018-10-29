#pragma once

#include "Light.h"

namespace rt {

class RAYLIB_API BackgroundLight : public ILight
{
public:
    BackgroundLight() = default;

    BackgroundLight(const math::Vector4& color)
        : color(color)
    {}

    math::Vector4 color;

    virtual const math::Box GetBoundingBox() const override;
    virtual bool TestRayHit(const math::Ray& ray, Float& outDistance) const override;
    virtual const Color Illuminate(const math::Vector4& scenePoint, RenderingContext& context, math::Vector4& outDirectionToLight, float& outDistance, float& outDirectPdfW) const override;
    virtual const Color GetRadiance(RenderingContext& context, const math::Vector4& rayDirection, const math::Vector4& hitPoint, Float* outDirectPdfA) const override;
    virtual bool IsFinite() const override final;
    virtual bool IsDelta() const override final;
};

} // namespace rt