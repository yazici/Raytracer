#pragma once

#include "../RayLib.h"

#include "Context.h"
#include "Counters.h"
#include "PostProcess.h"

#include "../Math/Random.h"
#include "../Sampling/HaltonSampler.h"
#include "../Sampling/GenericSampler.h"
#include "../Math/Rectangle.h"
#include "../Utils/Bitmap.h"
#include "../Utils/ThreadPool.h"
#include "../Utils/AlignmentAllocator.h"


namespace rt {

class IRenderer;
class Camera;

using RendererPtr = std::shared_ptr<IRenderer>;

struct RenderingProgress
{
    Uint32 passesFinished = 0;
    Uint32 activePixels = 0;
    Uint32 activeBlocks = 0;
    float converged = 0.0f;
    float averageError = std::numeric_limits<float>::infinity();
};

class RT_ALIGN(64) Viewport : public Aligned<64>
{
public:
    RAYLIB_API Viewport();
    RAYLIB_API ~Viewport();

    RAYLIB_API bool Resize(Uint32 width, Uint32 height);
    RAYLIB_API bool SetRenderingParams(const RenderingParams& params);
    RAYLIB_API bool SetRenderer(const RendererPtr& renderer);
    RAYLIB_API bool SetPostprocessParams(const PostprocessParams& params);
    RAYLIB_API bool Render(const Camera& camera);
    RAYLIB_API void Reset();

    RAYLIB_API void SetPixelBreakpoint(Uint32 x, Uint32 y);

    RT_FORCE_INLINE const Bitmap& GetFrontBuffer() const { return mFrontBuffer; }
    RT_FORCE_INLINE const Bitmap& GetSumBuffer() const { return mSum; }

    RT_FORCE_INLINE Uint32 GetWidth() const { return mSum.GetWidth(); }
    RT_FORCE_INLINE Uint32 GetHeight() const { return mSum.GetHeight(); }

    RT_FORCE_INLINE const RenderingProgress& GetProgress() const { return mProgress; }
    RT_FORCE_INLINE const RayTracingCounters& GetCounters() const { return mCounters; }

    RAYLIB_API void VisualizeActiveBlocks(Bitmap& bitmap) const;

private:
    void InitThreadData();

    // region of a image used for adaptive rendering
    using Block = math::Rectangle<Uint32>;

    struct TileRenderingContext
    {
        const IRenderer& renderer;
        const Camera& camera;
        const math::Vector4 sampleOffset;
    };

    struct RT_ALIGN(16) PostprocessParamsInternal
    {
        PostprocessParams params;

        math::Vector4 colorScale = math::Vector4::Zero();
        bool fullUpdateRequired = false;
    };

    void BuildInitialBlocksList();

    // compute average error (variance) in the image
    void ComputeError();

    // calculate estimated error (variance) of a given block
    float ComputeBlockError(const Block& block) const;

    // generate list of tiles to be rendered (updates mRenderingTiles)
    void GenerateRenderingTiles();

    void UpdateBlocksList();

    // raytrace single image tile (will be called from multiple threads)
    void PreRenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, const Block& tile);
    void RenderTile(const TileRenderingContext& tileContext, RenderingContext& renderingContext, const Block& tile);

    void PerformPostProcess();

    // generate "front buffer" image from "sum" image
    void PostProcessTile(const Block& tile, Uint32 threadID);

    ThreadPool mThreadPool;

    RendererPtr mRenderer;

    math::Random mRandomGenerator;
    HaltonSequence mHaltonSequence;

    std::vector<GenericSampler, AlignmentAllocator<GenericSampler, 64>> mSamplers;
    std::vector<RenderingContext, AlignmentAllocator<RenderingContext, 64>> mThreadData;

    Bitmap mSum;            // image with accumulated samples (floating point, high dynamic range)
    Bitmap mSecondarySum;   // contains image with every second sample - required for adaptive rendering
    Bitmap mFrontBuffer;    // postprocesses image (low dynamic range)
    std::vector<Uint32> mPassesPerPixel;
    std::vector<math::Float2> mPixelSalt; // salt value for each pixel

    RenderingParams mParams;
    PostprocessParamsInternal mPostprocessParams;

    RayTracingCounters mCounters;

    RenderingProgress mProgress;

    std::vector<Block> mBlocks;
    std::vector<Block> mRenderingTiles;

    PixelBreakpoint mPendingPixelBreakpoint;
};

} // namespace rt
