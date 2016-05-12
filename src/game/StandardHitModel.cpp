#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

StandardHitModel::StandardHitModel( Client& client_, vitality_t vitality_ )
    : AbstractStateHitModel ( TYPE_STANDARD, client_, vitality_ )
    , _torsoAdjust          ( 0.0f )
{
}

///////////////////////////////////////////////////////////////////////////////

StandardHitModel::~StandardHitModel()
{
}

//////////////////////////////////////////////////////////////////////////////

void
StandardHitModel::doState()
{
    switch (_state) {
        case STATE_CROUCH:
            _footBox.unset( HVF_ENABLED );
            _torsoAdjust = 0.0f;
            break;

        case STATE_DEAD:
        case STATE_PLAYDEAD:
            _footBox.set( HVF_ENABLED );
            _torsoAdjust = -4.0f;
            break;

        default:
        case STATE_STAND:
            _footBox.unset( HVF_ENABLED );
            _torsoAdjust = 0.0f; //-10.5f;  // same or less than head Z-dimension
            break;

        case STATE_PRONE:
            _footBox.set( HVF_ENABLED );
            _torsoAdjust = -4.0f;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
StandardHitModel::doStateRun()
{
    // Update head box.
    {
        grefEntity_t re;
        mdx_gentity_to_grefEntity( &client.gentity, &re, time );

        orientation_t orient;
        mdx_head_position( &client.gentity, &re, orient.origin );

        VectorSet( _headBox.mins, -6.0f, -6.0f, -6.0f );
        VectorSet( _headBox.maxs,  6.0f,  6.0f,  6.0f );

        VectorAdd( _headBox.mins, orient.origin, _headBox.mins );
        VectorAdd( _headBox.maxs, orient.origin, _headBox.maxs );
    }

    // Update torso box.
    {
        VectorCopy( client.gentity.r.currentOrigin, _torsoBox.mins );
        VectorAdd( _torsoBox.mins, client.gentity.r.mins, _torsoBox.mins );

        VectorCopy( client.gentity.r.currentOrigin, _torsoBox.maxs );
        VectorAdd( _torsoBox.maxs, client.gentity.r.maxs, _torsoBox.maxs );
        _torsoBox.maxs[2] += _torsoAdjust;
    }

    // Update legs box.
    {
        vec3_t flatforward;
        AngleVectors( client.gclient.ps.viewangles, flatforward, NULL, NULL );
        flatforward[2] = 0;
        VectorNormalizeFast( flatforward );

        vec3_t origin;
        if ( client.gclient.ps.eFlags & EF_PRONE) {
            origin[0] = client.gentity.r.currentOrigin[0] + flatforward[0] * -32;
            origin[1] = client.gentity.r.currentOrigin[1] + flatforward[1] * -32;
        }
        else {
            origin[0] = client.gentity.r.currentOrigin[0] + flatforward[0] * 32;
            origin[1] = client.gentity.r.currentOrigin[1] + flatforward[1] * 32;
        }
        origin[2] = client.gentity.r.currentOrigin[2] + client.gclient.pmext.proneLegsOffset;

        VectorCopy( playerlegsProneMins, _footBox.mins );
        VectorAdd( _footBox.mins, origin, _footBox.mins );

        VectorCopy( playerlegsProneMaxs, _footBox.maxs );
        VectorAdd( _footBox.maxs, origin, _footBox.maxs );
    }
}

///////////////////////////////////////////////////////////////////////////////

StandardHitModel&
StandardHitModel::operator=( const StandardHitModel& obj )
{
    AbstractStateHitModel::operator=( obj );
    _torsoAdjust = obj._torsoAdjust;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

StandardHitModel*
StandardHitModel::doSnapshot()
{
    StandardHitModel* obj = new StandardHitModel( client, VITALITY_GHOST );
    *obj = *this;
    return obj;
}
