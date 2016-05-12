#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

EtmainHitModel::EtmainHitModel( Client& client_, vitality_t vitality_ )
    : AbstractStateHitModel ( TYPE_ETMAIN, client_, vitality_ )
{
}

///////////////////////////////////////////////////////////////////////////////

EtmainHitModel::~EtmainHitModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
EtmainHitModel::doState()
{
    switch (_state) {
        case STATE_CROUCH:
            _headBox.set( HVF_ENABLED );
            _footBox.unset( HVF_ENABLED );
            break;

        case STATE_DEAD:
        case STATE_PLAYDEAD:
            _headBox.unset( HVF_ENABLED );
            _footBox.unset( HVF_ENABLED );
            break;

        default:
        case STATE_STAND:
            _headBox.set( HVF_ENABLED );
            _footBox.unset( HVF_ENABLED );
            break;

        case STATE_PRONE:
            _headBox.set( HVF_ENABLED );
            _footBox.set( HVF_ENABLED );
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
EtmainHitModel::doStateRun()
{
    // Update head box.
    {
        vec3_t origin;
        VectorCopy( client.gentity.r.currentOrigin, origin );

        float tiltfactor = 5.0f;
        switch (_state) {
            case STATE_CROUCH:
                origin[2] += float(client.gclient.ps.crouchViewHeight - 12) / 2.0f;
                break;

            case STATE_PRONE:
                origin[2] += float(client.gclient.ps.viewheight - 56) / 2.0f;
                tiltfactor = 24.0f;
                break;

            case STATE_STAND:
                origin[2] += client.gclient.ps.viewheight / 2.0f;
                break;

            default:
                break;
        }

        vec3_t angles;
        VectorCopy( client.gclient.ps.viewangles, angles );
        if ( angles[PITCH] > 180.0f )
            angles[PITCH] = (-360.0f + angles[PITCH]) * 0.75f;
        else
            angles[PITCH] *= 0.75f;

        vec3_t forward, up;
        AngleVectors( angles, forward, NULL, up );

        vec3_t v;
        VectorScale( forward, tiltfactor, v );
        VectorMA( v, 18.0f, up, v );
        VectorAdd( origin, v, origin );

        VectorSet( _headBox.mins, -6, -6, -2 );
        VectorAdd( _headBox.mins, origin, _headBox.mins );

        VectorSet( _headBox.maxs, 6, 6, 10 );
        VectorAdd( _headBox.maxs, origin, _headBox.maxs );
    }

    // Update torso box.
    {
        VectorCopy( client.gentity.r.currentOrigin, _torsoBox.mins );
        VectorAdd( _torsoBox.mins, client.gentity.r.mins, _torsoBox.mins );

        VectorCopy( client.gentity.r.currentOrigin, _torsoBox.maxs );
        VectorAdd( _torsoBox.maxs, client.gentity.r.maxs, _torsoBox.maxs );
    }

    // Update legs box.
    {
        vec3_t flatforward;
        AngleVectors( client.gclient.ps.viewangles, flatforward, NULL, NULL );
        flatforward[2] = 0;
        VectorNormalizeFast( flatforward );

        vec3_t origin;
        origin[0] = client.gentity.r.currentOrigin[0] + flatforward[0] * -32;
        origin[1] = client.gentity.r.currentOrigin[1] + flatforward[1] * -32;
        origin[2] = client.gentity.r.currentOrigin[2] + client.gclient.pmext.proneLegsOffset;

        VectorCopy( playerlegsProneMins, _footBox.mins );
        VectorAdd( _footBox.mins, origin, _footBox.mins );

        VectorCopy( playerlegsProneMaxs, _footBox.maxs );
        VectorAdd( _footBox.maxs, origin, _footBox.maxs );
    }
}

///////////////////////////////////////////////////////////////////////////////

EtmainHitModel&
EtmainHitModel::operator=( const EtmainHitModel& obj )
{
    AbstractStateHitModel::operator=( obj );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

EtmainHitModel*
EtmainHitModel::doSnapshot()
{
    EtmainHitModel* obj = new EtmainHitModel( client, VITALITY_GHOST );
    *obj = *this;
    return obj;
}
