#include "PCH.h"
#include "DielectricBSDF.h"
#include "../Material.h"
#include "Math/Random.h"
#include "Math/Utils.h"

namespace rt {

using namespace math;

const char* DielectricBSDF::GetName() const
{
    return "dielectric";
}

bool DielectricBSDF::Sample(SamplingContext& ctx) const
{
    const float NdotV = ctx.outgoingDir.z;
    if (Abs(NdotV) < CosEpsilon)
    {
        return false;
    }

    float ior = ctx.materialParam.IoR;
    bool fallbackToSingleWavelength = false;

    // handle dispersion
#ifdef RT_ENABLE_SPECTRAL_RENDERING
    if (ctx.material.isDispersive)
    {
        const float lambda = 1.0e+6f * Wavelength::Lower + ctx.wavelength.GetBase() * (Wavelength::Higher - Wavelength::Lower);
        // Cauchy's equation for light dispersion
        const float lambda2 = lambda * lambda;
        const float lambda4 = lambda2 * lambda2;
        ior += ctx.material.dispersionParams.C / lambda2;
        ior += ctx.material.dispersionParams.D / lambda4;
        if (!ctx.wavelength.isSingle)
        {
            fallbackToSingleWavelength = true;
            ctx.wavelength.isSingle = true;
        }
    }
#endif // RT_ENABLE_SPECTRAL_RENDERING

    // compute Fresnel term
    const float F = FresnelDielectric(NdotV, ior);
    const bool reflection = ctx.randomGenerator.GetFloat() < F;

    if (reflection) 
    {
        ctx.outIncomingDir = -Vector4::Reflect3(ctx.outgoingDir, VECTOR_Z);
        ctx.outEventType = SpecularReflectionEvent;
    }
    else // transmission
    {
        ctx.outIncomingDir = Vector4::Refract3(-ctx.outgoingDir, VECTOR_Z, ior);
        ctx.outEventType = SpecularRefractionEvent;
    }

    const float NdotL = ctx.outIncomingDir.z;

    if ((NdotV * NdotL > 0.0f) != reflection)
    {
        // discard samples that land on wrong surface side
        return false;
    }

    if (reflection)
    {
        ctx.outPdf = F;
        ctx.outColor = Color::One();
    }
    else
    {
        ctx.outPdf = 1.0f - F;
        ctx.outColor = ctx.materialParam.baseColor;
    }

    if (fallbackToSingleWavelength)
    {
        // in case of wavelength-dependent event, switch to single wavelength per ray
        ctx.outColor *= Color::SingleWavelengthFallback();
    }

    return true;
}

const Color DielectricBSDF::Evaluate(const EvaluationContext& ctx, Float* outDirectPdfW) const
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