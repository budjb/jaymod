#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

OrientedHitModel::OrientedHitModel( Client& client_, vitality_t vitality_ )
    : AbstractHitModel ( TYPE_ORIENTED, client_, vitality_ )

    , _headBox      ( AbstractHitVolume::ZONE_HEAD,       *this, AbstractHitVolume::SCOPE_PLAYER )
    , _armLeftBox   ( AbstractHitVolume::ZONE_ARM_LEFT,   *this, AbstractHitVolume::SCOPE_PLAYER )
    , _armRightBox  ( AbstractHitVolume::ZONE_ARM_RIGHT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _handLeftBox  ( AbstractHitVolume::ZONE_HAND_LEFT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _handRightBox ( AbstractHitVolume::ZONE_HAND_RIGHT, *this, AbstractHitVolume::SCOPE_PLAYER )
    , _torsoBox     ( AbstractHitVolume::ZONE_TORSO,      *this, AbstractHitVolume::SCOPE_PLAYER )
    , _legLeftBox   ( AbstractHitVolume::ZONE_LEG_LEFT,   *this, AbstractHitVolume::SCOPE_PLAYER )
    , _legRightBox  ( AbstractHitVolume::ZONE_LEG_RIGHT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _footLeftBox  ( AbstractHitVolume::ZONE_FOOT_LEFT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _footRightBox ( AbstractHitVolume::ZONE_FOOT_RIGHT, *this, AbstractHitVolume::SCOPE_PLAYER )
{
}

///////////////////////////////////////////////////////////////////////////////

OrientedHitModel::~OrientedHitModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
OrientedHitModel::doRun()
{
    // Fetch origins/orientations for player model.   
    grefEntity_t re;
    mdx_gentity_to_grefEntity( &client.gentity, &re, time );

    vec3_t        origins[MRP_MAX];
    orientation_t orients[MRP_MAX];
    mdx_advanced_positions( client.gentity, re, origins, orients );

    // Apply world axis to local axis we need for this function body.
    {
        vec3_t axis[3];
        MatrixMultiply( re.headAxis, orients[MRP_NECK].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_NECK].axis );

        MatrixMultiply( re.headAxis, orients[MRP_ELBOW_LEFT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_ELBOW_LEFT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_HAND_LEFT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_HAND_LEFT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_ELBOW_RIGHT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_ELBOW_RIGHT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_HAND_RIGHT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_HAND_RIGHT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_CHEST].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_CHEST].axis );

        MatrixMultiply( re.headAxis, orients[MRP_PELVIS].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_PELVIS].axis );

        MatrixMultiply( re.headAxis, orients[MRP_KNEE_LEFT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_KNEE_LEFT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_KNEE_RIGHT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_KNEE_RIGHT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_ANKLE_LEFT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_ANKLE_LEFT].axis );

        MatrixMultiply( re.headAxis, orients[MRP_ANKLE_RIGHT].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_ANKLE_RIGHT].axis );
    }

    vec3_t shoulderLeftOrigin;
    VectorMA( origins[MRP_CHEST], 6.0f, orients[MRP_CHEST].axis[2], shoulderLeftOrigin );
    VectorMA( shoulderLeftOrigin, 8.0f, orients[MRP_CHEST].axis[1], shoulderLeftOrigin );
    
    vec3_t shoulderRightOrigin;
    VectorMA( origins[MRP_CHEST], 6.0f, orients[MRP_CHEST].axis[2], shoulderRightOrigin );
    VectorMA( shoulderRightOrigin, -8.0f, orients[MRP_CHEST].axis[1], shoulderRightOrigin );

    vec3_t hipLeftOrigin;
    VectorMA( origins[MRP_PELVIS], 8.0f, orients[MRP_PELVIS].axis[1], hipLeftOrigin );

    vec3_t hipRightOrigin;
    VectorMA( origins[MRP_PELVIS], -8.0f, orients[MRP_PELVIS].axis[1], hipRightOrigin );

    // Update head box.
    {
        // MODEL: head.md3
        // bboxMin:   -4.125000   -3.609375   -2.781250
        // bboxMax:    5.187500    3.718750   10.406250
        // localOrigin: 0.000000 0.000000 0.000000
        // radius: 12.207756

        memcpy( _headBox.axis, orients[MRP_NECK].axis, sizeof(_headBox.axis) );

        VectorCopy( origins[MRP_NECK], _headBox.origin );
        VectorSet( _headBox.scale, 9.3125f, 7.328125, 11.187500 );

        VectorMA( _headBox.origin, -4.6562500f, _headBox.axis[0], _headBox.origin );
        VectorMA( _headBox.origin, -3.6640625f, _headBox.axis[1], _headBox.origin );

        _headBox.reorient();
    }

    // Update hand-left box.
    {
        vec3_t forearm;
        VectorSubtract( origins[MRP_ELBOW_LEFT], origins[MRP_HAND_LEFT], forearm );

        vec3_t angles;
        vectoangles( forearm, angles );
        AnglesToAxis( angles, _handLeftBox.axis );

        VectorCopy( origins[MRP_HAND_LEFT], _handLeftBox.origin );
        VectorSet( _handLeftBox.scale, 18.0f, 6.0f, 6.0f );

        VectorMA( _handLeftBox.origin, -2.0f, _handLeftBox.axis[0], _handLeftBox.origin );
        VectorMA( _handLeftBox.origin, -3.0f, _handLeftBox.axis[1], _handLeftBox.origin );
        VectorMA( _handLeftBox.origin, -3.0f, _handLeftBox.axis[2], _handLeftBox.origin );

        _handLeftBox.reorient();
    }

    // Update hand-right box.
    {
        vec3_t forearm;
        VectorSubtract( origins[MRP_ELBOW_RIGHT], origins[MRP_HAND_RIGHT], forearm );

        vec3_t angles;
        vectoangles( forearm, angles );
        AnglesToAxis( angles, _handRightBox.axis );

        VectorCopy( origins[MRP_HAND_RIGHT], _handRightBox.origin );
        VectorSet( _handRightBox.scale, 18.0f, 6.0f, 6.0f );

        VectorMA( _handRightBox.origin, -2.0f, _handRightBox.axis[0], _handRightBox.origin );
        VectorMA( _handRightBox.origin, -3.0f, _handRightBox.axis[1], _handRightBox.origin );
        VectorMA( _handRightBox.origin, -3.0f, _handRightBox.axis[2], _handRightBox.origin );

        _handRightBox.reorient();
    }

    // Update arm-left box.
    {
        vec3_t arm;
        VectorSubtract( shoulderLeftOrigin, origins[MRP_ELBOW_LEFT], arm );

        vec3_t angles;
        vectoangles( arm, angles );
        AnglesToAxis( angles, _armLeftBox.axis );

        VectorCopy( origins[MRP_ELBOW_LEFT], _armLeftBox.origin );
        VectorSet( _armLeftBox.scale, 14.0f, 7.0f, 7.0f );

        VectorMA( _armLeftBox.origin, -2.0f, _armLeftBox.axis[0], _armLeftBox.origin );
        VectorMA( _armLeftBox.origin, -3.5f, _armLeftBox.axis[1], _armLeftBox.origin );
        VectorMA( _armLeftBox.origin, -3.5f, _armLeftBox.axis[2], _armLeftBox.origin );

        _armLeftBox.reorient();
    }

    // Update arm-right box.
    {
        vec3_t arm;
        VectorSubtract( shoulderRightOrigin, origins[MRP_ELBOW_RIGHT], arm );
    
        vec3_t angles;
        vectoangles( arm, angles );
        AnglesToAxis( angles, _armRightBox.axis );

        VectorCopy( origins[MRP_ELBOW_RIGHT], _armRightBox.origin );
        VectorSet( _armRightBox.scale, 14.0f, 7.0f, 7.0f );

        VectorMA( _armRightBox.origin, -2.0f, _armRightBox.axis[0], _armRightBox.origin );
        VectorMA( _armRightBox.origin, -3.5f, _armRightBox.axis[1], _armRightBox.origin );
        VectorMA( _armRightBox.origin, -3.5f, _armRightBox.axis[2], _armRightBox.origin );

        _armRightBox.reorient();
    }

    // Update foot-left box.
    {
        vec3_t shin;
        VectorSubtract( origins[MRP_KNEE_LEFT], origins[MRP_ANKLE_LEFT], shin );

        vec3_t angles;
        vectoangles( shin, angles );
        AnglesToAxis( angles, _footLeftBox.axis );

        VectorCopy( origins[MRP_ANKLE_LEFT], _footLeftBox.origin );
        VectorSet( _footLeftBox.scale, 21.0f, 8.0f, 8.0f );

        VectorMA( _footLeftBox.origin, -5.0f, _footLeftBox.axis[0], _footLeftBox.origin );
        VectorMA( _footLeftBox.origin, -4.0f, _footLeftBox.axis[1], _footLeftBox.origin );
        VectorMA( _footLeftBox.origin, -4.0f, _footLeftBox.axis[2], _footLeftBox.origin );

        _footLeftBox.reorient();
    }

    // Update foot-right box.
    {
        vec3_t shin;
        VectorSubtract( origins[MRP_KNEE_RIGHT], origins[MRP_ANKLE_RIGHT], shin );

        vec3_t angles;
        vectoangles( shin, angles );
        AnglesToAxis( angles, _footRightBox.axis );

        VectorCopy( origins[MRP_ANKLE_RIGHT], _footRightBox.origin );
        VectorSet( _footRightBox.scale, 21.0f, 8.0f, 8.0f );

        VectorMA( _footRightBox.origin, -5.0f, _footRightBox.axis[0], _footRightBox.origin );
        VectorMA( _footRightBox.origin, -4.0f, _footRightBox.axis[1], _footRightBox.origin );
        VectorMA( _footRightBox.origin, -4.0f, _footRightBox.axis[2], _footRightBox.origin );

        _footRightBox.reorient();
    }

    // Update leg-left box.
    {
        vec3_t leg;
        VectorSubtract( hipLeftOrigin, origins[MRP_KNEE_LEFT], leg );

        vec3_t angles;
        vectoangles( leg, angles );
        AnglesToAxis( angles, _legLeftBox.axis );

        VectorCopy( origins[MRP_KNEE_LEFT], _legLeftBox.origin );
        VectorSet( _legLeftBox.scale, 15.0f, 10.0f, 10.0f );

        VectorMA( _legLeftBox.origin, -1.0f, _legLeftBox.axis[0], _legLeftBox.origin );
        VectorMA( _legLeftBox.origin, -5.0f, _legLeftBox.axis[1], _legLeftBox.origin );
        VectorMA( _legLeftBox.origin, -5.0f, _legLeftBox.axis[2], _legLeftBox.origin );

        _legLeftBox.reorient();
    }

    // Update leg-right box.
    {
        vec3_t leg;
        VectorSubtract( hipRightOrigin, origins[MRP_KNEE_RIGHT], leg );

        vec3_t angles;
        vectoangles( leg, angles );
        AnglesToAxis( angles, _legRightBox.axis );

        VectorCopy( origins[MRP_KNEE_RIGHT], _legRightBox.origin );
        VectorSet( _legRightBox.scale, 15.0f, 10.0f, 10.0f );

        VectorMA( _legRightBox.origin, -1.0f, _legRightBox.axis[0], _legRightBox.origin );
        VectorMA( _legRightBox.origin, -5.0f, _legRightBox.axis[1], _legRightBox.origin );
        VectorMA( _legRightBox.origin, -5.0f, _legRightBox.axis[2], _legRightBox.origin );

        _legRightBox.reorient();
    }

    // Update torso box.
    {
        vec3_t spine;
        VectorSubtract( origins[MRP_PELVIS], origins[MRP_NECK], spine );

#if 0
        vec3_t spineAngles;
        vectoangles( spine, spineAngles );

        vec3_t chestAngles;
        AxisToAngles( orients[MRP_CHEST].axis, chestAngles );

        spineAngles[ROLL] = chestAngles[ROLL];
        AnglesToAxis( spineAngles, _torsoBox.axis );
#endif
        vec3_t angles;
        vectoangles( spine, angles );
        AnglesToAxis( angles, _torsoBox.axis );

        VectorCopy( origins[MRP_NECK], _torsoBox.origin );
        VectorSet( _torsoBox.scale, 32.0f, 18.0f, 12.0f );

        VectorMA( _torsoBox.origin, -1.0f, _torsoBox.axis[0], _torsoBox.origin );
        VectorMA( _torsoBox.origin, -9.0f, _torsoBox.axis[1], _torsoBox.origin );
        VectorMA( _torsoBox.origin, -6.0f, _torsoBox.axis[2], _torsoBox.origin );

        _torsoBox.reorient();
    }
}

///////////////////////////////////////////////////////////////////////////////

OrientedHitModel&
OrientedHitModel::operator=( const OrientedHitModel& obj )
{
    AbstractHitModel::operator=( obj );

    _headBox      = obj._headBox;
    _armLeftBox   = obj._armLeftBox;
    _armRightBox  = obj._armRightBox;
    _handLeftBox  = obj._handLeftBox;
    _handRightBox = obj._handRightBox;
    _torsoBox     = obj._torsoBox;
    _legLeftBox   = obj._legLeftBox;
    _legRightBox  = obj._legRightBox;
    _footLeftBox  = obj._footLeftBox;
    _footRightBox = obj._footRightBox;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

OrientedHitModel*
OrientedHitModel::doSnapshot()
{
    OrientedHitModel* obj = new OrientedHitModel( client, VITALITY_GHOST );
    *obj = *this;
    return obj;
}
