#ifndef GAME_ABSTRACTHITVOLUME_H
#define GAME_ABSTRACTHITVOLUME_H

///////////////////////////////////////////////////////////////////////////////

class AbstractHitModel;

///////////////////////////////////////////////////////////////////////////////

/*
 * HitVolume is an abstract class which represents a volume of some kind.
 *
 * To play nice with the game engine, all HVs have an axis-aligned bounding-box
 * which is defined by public fields { mins, maxs }. External code such as a
 * HitModel is responsible for updating these fields directly.
 *
 * An HV is attributed with a scope such as WORLD or PLAYER which indcates
 * when it is appropriate to perform ray-tracing. For example, a WORLD
 * scope indicates the HV is suitable to be used when tracing against the
 * entire 3D world. A PLAYER scope means the world-trace has been completed,
 * and now we're doing a sub-trace against HV which represent the finer points
 * of a player's hit-model.
 *
 */
class AbstractHitVolume
{
public:
    enum type_t {
        _TYPE_UNDEFINED,

        TYPE_ALIGNED_CUBOID,
        TYPE_ORIENTED_CUBOID,
        TYPE_SPHERE,
        TYPE_CYLINDER,

        _TYPE_MAX,
    };

    enum scope_t {
        _SCOPE_UNDEFINED,

        SCOPE_WORLD,
        SCOPE_PLAYER,

        _SCOPE_MAX,
    };

    enum zone_t {
        _ZONE_UNDEFINED,

        ZONE_BODY,
        ZONE_HEAD,
        ZONE_ARM_LEFT,
        ZONE_ARM_RIGHT,
        ZONE_HAND_LEFT,
        ZONE_HAND_RIGHT,
        ZONE_TORSO,
        ZONE_TORSO_LEFT,
        ZONE_TORSO_RIGHT,
        ZONE_LEGS,
        ZONE_LEG_LEFT,
        ZONE_LEG_RIGHT,
        ZONE_FOOT_LEFT,
        ZONE_FOOT_RIGHT,

        _ZONE_MAX,
    };

    static string  toString( type_t );
    static string& toString( type_t, string& );

    static string  toString( scope_t );
    static string& toString( scope_t, string& );

    static string  toString( zone_t );
    static string& toString( zone_t, string& );

protected:
    AbstractHitVolume( type_t, zone_t, entityType_t, AbstractHitModel&, scope_t );

    AbstractHitModel& _hitModel;
    gentity_t*        _entity;
    int               _flags;

    virtual void doEntityCompute() = 0;

public:
    virtual ~AbstractHitVolume();

    virtual AbstractHitVolume& operator=( const AbstractHitVolume& );

    virtual bool castRay( TraceContext&, vec3_t&, float& ) = 0;

    void entityAlloc   ( );
    void entityCompute ( );
    void entityFree    ( );
    void expand        ( const AbstractHitVolume& );
    void lerp          ( AbstractHitVolume&, float );
    void recordHit     ( );
    void set           ( hitVolumeFlags_t );
    void set           ( hitVolumeFlags_t, bool );
    void unset         ( hitVolumeFlags_t );

    const type_t       type;
    const scope_t      scope;
    const zone_t       zone;
    const entityType_t entityType;
    const int&         flags;

    vec3_t mins;  // axis-aligned bounding-box, absolute world space
    vec3_t maxs;  // axis-aligned bounding-box, absolute world space
};

///////////////////////////////////////////////////////////////////////////////

#include <game/AlignedCuboidHV.h>
#include <game/OrientedCuboidHV.h>

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ABSTRACTHITVOLUME_H
