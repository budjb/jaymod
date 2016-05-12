#include <bgame/impl.h>

namespace molotov {

///////////////////////////////////////////////////////////////////////////////

const vec3_t mins = { -2.108, -2.108, -4.188 };
const vec3_t maxs = {  2.108,  2.108, 11.057 };

const float chunkRadius     = 125.0f;              // firechunk radius
const float chunkRadius_2   = chunkRadius * 2.0f;  // convenience
const float chunkHop        = chunkRadius / 5.0f;  // hop for next chunk
const float chunkSpread     = chunkRadius / 2.0f;  // maximum spread variance of each chunk
const int   chunkDuration_i = 3000;                // lifetime for a chunk
const float chunkDuration_f = chunkDuration_i;     // lifetime for a chunk
const float chunkNumFrames  = 45;                  // number of visual frames (textures)
const int   chunkMax        = 6;                   // total num of chunks

const int screamDuration = 1700;  // duration of a scream

///////////////////////////////////////////////////////////////////////////////

} // namespace molotov
