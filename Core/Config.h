#pragma once

// enables runtime counting of ray-triangle and ray-box intersection tests
//#define RT_ENABLE_INTERSECTION_COUNTERS

// enables code for collecting path tracing debug data
#define RT_ENABLE_PATH_DEBUGGING

// enables spectral rendering via Monte Carlo wavelength sampling
// NOTE: this slows down everything significantly
//#define RT_ENABLE_SPECTRAL_RENDERING