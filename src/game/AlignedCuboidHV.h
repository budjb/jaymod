#ifndef GAME_ALIGNEDCUBOIDHV_H
#define GAME_ALIGNEDCUBOIDHV_H

///////////////////////////////////////////////////////////////////////////////

class AlignedCuboidHV : public AbstractHitVolume
{
protected:
    bool castRayPlane(
        TraceContext&,  // standard trace context
        vec3_t,         // result: position of hit
        float&,         // result: length of hit-ray
        const string&,  // name of face/plane
        vec3_t*,        // 4 verts defining in mins/maxs order
        int,            // which of 3-dims used for x-bounds check
        int );          // which of 3-dims used for y-bounds check

    void doEntityCompute();

public:
    AlignedCuboidHV( zone_t, AbstractHitModel&, scope_t );
    ~AlignedCuboidHV();

    bool castRay( TraceContext&, vec3_t&, float& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ALIGNEDCUBOIDHV_H
