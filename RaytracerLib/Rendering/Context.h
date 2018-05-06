#pragma once

#include "Counters.h"

#include "../Traversal/RayPacket.h"
#include "../Traversal/HitPoint.h"

#include "../Math/Random.h"

namespace rt {

enum class RenderingMode : Uint8
{
    Regular = 0,

    // geometry
    Depth,                      // visualize depth
    Position,                   // visualize world-space position
    Normals,                    // visualize normal vectors (in world space)
    Tangents,
    TexCoords,

    // material
    BaseColor,                  // visualize base color of the first intersection

    // stats
    RayBoxIntersection,         // visualize number of performed ray-box intersections
    RayBoxIntersectionPassed,   // visualize number of passed ray-box intersections
    RayTriIntersection,         // visualize number of performed ray-triangle intersections
    RayTriIntersectionPassed,   // visualize number of passed ray-triangle intersections
};

struct RenderingParams
{
    // Antialiasing factor: 1.0 is most optimal
    // Setting to values above 1 will blur the image
    Float antiAliasingSpread;

    // number of primary rays to be generated for image pixel
    Uint32 samplesPerPixel;

    // maximum ray depth
    Uint32 maxRayDepth;

    // ray depth after which Russian Roulette algorithm kicks in
    Uint32 minRussianRouletteDepth;

    // allows to enable debug rendering mode
    RenderingMode renderingMode;

    RenderingParams()
        : maxRayDepth(20)
        , minRussianRouletteDepth(2)
        , samplesPerPixel(1)
        , antiAliasingSpread(1.2f) // blur a little bit - real images are not perfectly sharp
        , renderingMode(RenderingMode::Regular)
    { }
};

/**
 * A structure with local (per-thread) data.
 * It's like a hub for all global params (read only) and local state (read write).
 */
struct RenderingContext
{
    // global rendering parameters
    const RenderingParams& params;

    // per-thread pseudo-random number generator
    math::Random& randomGenerator;

    // per-thread counters
    RayTracingCounters& counters;

    // for motion blur sampling
    float time;

    // counters used in local ray traversal routines
    LocalCounters localCounters;

    RayPacket rayPacket;

    HitPoint_Packet hitPoints;

    RenderingContext(math::Random& randomGenerator, const RenderingParams& params, RayTracingCounters& counters)
        : params(params)
        , randomGenerator(randomGenerator)
        , counters(counters)
    { }
};


} // namespace rt