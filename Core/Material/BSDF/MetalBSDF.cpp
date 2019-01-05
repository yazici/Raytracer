#include "PCH.h"
#include "MetalBSDF.h"
#include "../Material.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

const char* MetalBSDF::GetName() const
{
    return "metal";
}

bool MetalBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (NdotV < CosEpsilon)
    {
        return false;
    }

    // TODO
    // This is wrong!
    // IoR depends on the wavelength and this is the source of metal color.
    // Metal always reflect 100% pure white at grazing angle.
    const float F = FresnelMetal(NdotV, ctx.material.IoR, ctx.material.K);

    ctx.outColor = ctx.materialParam.baseColor * Color(F);
    ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
    ctx.outPdf = 1.0f;
    ctx.outEventType = SpecularReflectionEvent;

    return true;
}

const Color MetalBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
{
    RT_UNUSED(ctx);

    // Dirac delta, assume we cannot hit it

    if (outDirectPdfW)
    {
        *outDirectPdfW = 0.0f; 
    }

    return Color::Zero();
}

} // namespace rt