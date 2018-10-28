#pragma once

#include "BSDF.h"

namespace rt {

class OrenNayarBSDF : public BSDF
{
public:
    virtual bool Sample(SamplingContext& ctx) const override;
    virtual const math::Vector4 Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW = nullptr) const override;
};

} // namespace rt
