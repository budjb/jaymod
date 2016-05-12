#include <bgame/impl.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

void
__fastAddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs )
{
    if (v[0] < mins[0])
        mins[0] = v[0];
    else if (v[0] > maxs[0])
        maxs[0] = v[0];

    if (v[1] < mins[1])
        mins[1] = v[1];
    else if (v[1] > maxs[1])
        maxs[1] = v[1];

    if (v[2] < mins[2])
        mins[2] = v[2];
    else if (v[2] > maxs[2])
        maxs[2] = v[2];
}

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

AdvancedHitModel::AdvancedHitModel( Client& client_, vitality_t vitality_ )
    : AbstractHitModel ( TYPE_ADVANCED, client_, vitality_ )
    , _headBox         ( AbstractHitVolume::ZONE_HEAD,       *this, AbstractHitVolume::SCOPE_PLAYER )
    , _armLeftBox      ( AbstractHitVolume::ZONE_ARM_LEFT,   *this, AbstractHitVolume::SCOPE_PLAYER )
    , _armRightBox     ( AbstractHitVolume::ZONE_ARM_RIGHT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _handLeftBox     ( AbstractHitVolume::ZONE_HAND_LEFT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _handRightBox    ( AbstractHitVolume::ZONE_HAND_RIGHT, *this, AbstractHitVolume::SCOPE_PLAYER )
    , _torsoLeftBox    ( AbstractHitVolume::ZONE_TORSO_LEFT, *this, AbstractHitVolume::SCOPE_PLAYER )
    , _torsoRightBox   ( AbstractHitVolume::ZONE_TORSO_LEFT, *this, AbstractHitVolume::SCOPE_PLAYER )
    , _footLeftBox     ( AbstractHitVolume::ZONE_FOOT_LEFT,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _footRightBox    ( AbstractHitVolume::ZONE_FOOT_RIGHT, *this, AbstractHitVolume::SCOPE_PLAYER )
{
}

///////////////////////////////////////////////////////////////////////////////

AdvancedHitModel::~AdvancedHitModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
AdvancedHitModel::doRun()
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

        MatrixMultiply( re.torsoAxis, orients[MRP_BACK].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_BACK].axis );

        MatrixMultiply( re.torsoAxis, orients[MRP_CHEST].axis, axis );
        MatrixMultiply( axis, re.axis, orients[MRP_CHEST].axis );
    }

    const float fat = cvars::g_hitmodeFat.fvalue;
    const float sfat = fat / 2.0f;

    // Compute point offset from back for upperback.
    vec3_t backOffsetPoint;
    VectorMA( origins[MRP_BACK], -8.0f, orients[MRP_BACK].axis[0], backOffsetPoint );

    // Compute point offset for tailbone.
    vec3_t tailboneOffsetPoint, v;
    VectorSubtract( origins[MRP_PELVIS], origins[MRP_NECK], v );
    VectorNormalize( v );
    VectorMA( origins[MRP_PELVIS], 10.0f, v, tailboneOffsetPoint );

    // Compute point offsets from chest for stomach, hip-left and hip-right.
    vec3_t stomachOffsetPoint;
    VectorMA( origins[MRP_CHEST], 10.0f + sfat, orients[MRP_CHEST].axis[0], stomachOffsetPoint );

    vec3_t hipLeftOffsetPoint;
    VectorMA( tailboneOffsetPoint, 12.0f + fat, orients[MRP_CHEST].axis[1], hipLeftOffsetPoint );

    vec3_t hipRightOffsetPoint;
    VectorMA( tailboneOffsetPoint, -(12.0f + fat), orients[MRP_CHEST].axis[1], hipRightOffsetPoint );

    vec3_t shoulderLeftOffsetPoint;
    VectorMA( origins[MRP_CHEST], 6.0f, orients[MRP_CHEST].axis[2], shoulderLeftOffsetPoint );
    VectorMA( shoulderLeftOffsetPoint, 8.0f, orients[MRP_CHEST].axis[1], shoulderLeftOffsetPoint );

    vec3_t shoulderRightOffsetPoint;
    VectorMA( origins[MRP_CHEST], 6.0f, orients[MRP_CHEST].axis[2], shoulderRightOffsetPoint );
    VectorMA( shoulderRightOffsetPoint, -8.0f, orients[MRP_CHEST].axis[1], shoulderRightOffsetPoint );

    // Update head box.
    {
        // MODEL: head.md3
        // bboxMin:   -4.125000   -3.609375   -2.781250
        // bboxMax:    5.187500    3.718750   10.406250
        // localOrigin: 0.000000 0.000000 0.000000
        // radius: 12.207756

        vec3_t vHead;
        VectorCopy( origins[MRP_NECK], vHead );
        VectorMA( vHead, 6.590334f, orients[MRP_NECK].axis[2], vHead );

        VectorSet( _headBox.mins, -4.656250f, -3.664062f, -6.593750f );
        VectorSet( _headBox.maxs,  4.656250f,  3.664062f,  6.593750f );

        VectorAdd( _headBox.mins, vHead, _headBox.mins );
        VectorAdd( _headBox.maxs, vHead, _headBox.maxs );
    }

    // Update arm-left box.
    {
        VectorSet( _armLeftBox.mins, -4.0f, -4.0f, -4.0f );
        VectorSet( _armLeftBox.maxs,  4.0f,  4.0f,  4.0f );

        VectorAdd( _armLeftBox.mins, shoulderLeftOffsetPoint, _armLeftBox.mins );
        VectorAdd( _armLeftBox.maxs, shoulderLeftOffsetPoint, _armLeftBox.maxs );

        // Expand to include elbow point.
        __fastAddPointToBounds( origins[MRP_ELBOW_LEFT], _armLeftBox.mins, _armLeftBox.maxs );
    }

    // Update arm-right box.
    {
        VectorSet( _armRightBox.mins, -4.0f, -4.0f, -4.0f );
        VectorSet( _armRightBox.maxs,  4.0f,  4.0f,  4.0f );

        VectorAdd( _armRightBox.mins, shoulderRightOffsetPoint, _armRightBox.mins );
        VectorAdd( _armRightBox.maxs, shoulderRightOffsetPoint, _armRightBox.maxs );

        // Expand to include elbow point.
        __fastAddPointToBounds( origins[MRP_ELBOW_RIGHT], _armRightBox.mins, _armRightBox.maxs );
    }

    // Update hand-left box.
    {
        VectorSet( _handLeftBox.mins, -4.0f, -4.0f, -4.0f );
        VectorSet( _handLeftBox.maxs,  4.0f,  4.0f,  4.0f );

        VectorAdd( _handLeftBox.mins, origins[MRP_HAND_LEFT], _handLeftBox.mins );
        VectorAdd( _handLeftBox.maxs, origins[MRP_HAND_LEFT], _handLeftBox.maxs );

        // Expand to include elbow point.
        __fastAddPointToBounds( origins[MRP_ELBOW_LEFT], _handLeftBox.mins, _handLeftBox.maxs );
    }

    // Update hand-right box.
    {
        VectorSet( _handRightBox.mins, -4.0f, -4.0f, -4.0f );
        VectorSet( _handRightBox.maxs,  4.0f,  4.0f,  4.0f );

        VectorAdd( _handRightBox.mins, origins[MRP_HAND_RIGHT], _handRightBox.mins );
        VectorAdd( _handRightBox.maxs, origins[MRP_HAND_RIGHT], _handRightBox.maxs );

        // Expand to include elbow point.
        __fastAddPointToBounds( origins[MRP_ELBOW_RIGHT], _handRightBox.mins, _handRightBox.maxs );
    }

    // Update torso-left box.
    {
        VectorCopy( origins[MRP_PELVIS], _torsoLeftBox.mins );
        VectorCopy( origins[MRP_PELVIS], _torsoLeftBox.maxs );

        // Expand to include neck, elbow knee, stomach and back points.
        __fastAddPointToBounds( origins[ MRP_NECK ],       _torsoLeftBox.mins, _torsoLeftBox.maxs );
        __fastAddPointToBounds( backOffsetPoint,           _torsoLeftBox.mins, _torsoLeftBox.maxs );
        __fastAddPointToBounds( stomachOffsetPoint,        _torsoLeftBox.mins, _torsoLeftBox.maxs );
        __fastAddPointToBounds( hipLeftOffsetPoint,        _torsoLeftBox.mins, _torsoLeftBox.maxs );
        __fastAddPointToBounds( origins[ MRP_KNEE_LEFT ],  _torsoLeftBox.mins, _torsoLeftBox.maxs );
    }

    // Update torso-right box.
    {
        VectorCopy( origins[MRP_PELVIS], _torsoRightBox.mins );
        VectorCopy( origins[MRP_PELVIS], _torsoRightBox.maxs );

        // Expand to include neck, elbow knee, stomach and back points.
        __fastAddPointToBounds( origins[ MRP_NECK ],        _torsoRightBox.mins, _torsoRightBox.maxs );
        __fastAddPointToBounds( backOffsetPoint,            _torsoRightBox.mins, _torsoRightBox.maxs );
        __fastAddPointToBounds( tailboneOffsetPoint,        _torsoRightBox.mins, _torsoRightBox.maxs );
        __fastAddPointToBounds( stomachOffsetPoint,         _torsoRightBox.mins, _torsoRightBox.maxs );
        __fastAddPointToBounds( hipRightOffsetPoint,        _torsoRightBox.mins, _torsoRightBox.maxs );
        __fastAddPointToBounds( origins[ MRP_KNEE_RIGHT ],  _torsoRightBox.mins, _torsoRightBox.maxs );
    }

    // Update foot-left box.
    {
        VectorSet( _footLeftBox.mins, -5.0f, -5.0f, -5.0f );
        VectorSet( _footLeftBox.maxs,  5.0f,  5.0f,  5.0f );

        VectorAdd( _footLeftBox.mins, origins[MRP_ANKLE_LEFT], _footLeftBox.mins );
        VectorAdd( _footLeftBox.maxs, origins[MRP_ANKLE_LEFT], _footLeftBox.maxs );

        // Expand to include knee point.
        __fastAddPointToBounds( origins[MRP_KNEE_LEFT], _footLeftBox.mins, _footLeftBox.maxs );
    }

    // Update foot-right box.
    {
        VectorSet( _footRightBox.mins, -5.0f, -5.0f, -5.0f );
        VectorSet( _footRightBox.maxs,  5.0f,  5.0f,  5.0f );

        VectorAdd( _footRightBox.mins, origins[MRP_ANKLE_RIGHT], _footRightBox.mins );
        VectorAdd( _footRightBox.maxs, origins[MRP_ANKLE_RIGHT], _footRightBox.maxs );

        // Expand to include knee point.
        __fastAddPointToBounds( origins[MRP_KNEE_RIGHT], _footRightBox.mins, _footRightBox.maxs );
    }
}

///////////////////////////////////////////////////////////////////////////////

AdvancedHitModel&
AdvancedHitModel::operator=( const AdvancedHitModel& obj )
{
    AbstractHitModel::operator=( obj );

    _headBox       = obj._headBox;
    _armLeftBox    = obj._armLeftBox;
    _armRightBox   = obj._armRightBox;
    _handLeftBox   = obj._handLeftBox;
    _handRightBox  = obj._handRightBox;
    _torsoLeftBox  = obj._torsoLeftBox;
    _torsoRightBox = obj._torsoRightBox;
    _footLeftBox   = obj._footLeftBox;
    _footRightBox  = obj._footRightBox;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

AdvancedHitModel*
AdvancedHitModel::doSnapshot()
{
    AdvancedHitModel* obj = new AdvancedHitModel( client, VITALITY_GHOST );
    *obj = *this;
    return obj;
}
