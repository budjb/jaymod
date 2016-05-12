#ifndef BGAME_MOLOTOV_H
#define BGAME_MOLOTOV_H

namespace molotov {

///////////////////////////////////////////////////////////////////////////////

extern const vec3_t mins; // 3rd-person model bounding-box
extern const vec3_t maxs;

extern const float chunkRadius;      // overall radius for each flame chunk
extern const float chunkRadius_2;    // radius*2
extern const float chunkHop;         // forward spread distance
extern const float chunkSpread;      // lateral spread distance (up/right)
extern const int   chunkDuration_i;  // length of exploision (ms)
extern const float chunkDuration_f;  // convenience
extern const float chunkNumFrames;   // number of fireball textures used in each chunk
extern const int   chunkMax;         // number of chunks for each explosion

extern const int   screamDuration;   // minimum time between screams

///////////////////////////////////////////////////////////////////////////////

} // namespace molotov

#endif // BGAME_MOLOTOV_H
