#ifndef GAME_ORIENTEDCUBOIDHV_H
#define GAME_ORIENTEDCUBOIDHV_H

///////////////////////////////////////////////////////////////////////////////

class OrientedCuboidHV : public AbstractHitVolume
{
protected:
    bool castRayTriangle(
        TraceContext&,  // standard trace context
        vec3_t&,        // result: position of hit
        float&,         // result: length of hit-ray
        const string&,  // name of face/plane
        const vec3_t&,
        const vec3_t&,
        const vec3_t& );

    void doEntityCompute();

public:
    OrientedCuboidHV( zone_t, AbstractHitModel&, scope_t );
    ~OrientedCuboidHV();

    OrientedCuboidHV& operator=( const OrientedCuboidHV& );

    bool castRay  ( TraceContext&, vec3_t&, float& );
    void reorient ( );

    vec3_t origin;
    vec3_t scale;
    vec3_t axis[3];
    vec3_t coords[8];
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ORIENTEDCUBOIDHV_H
