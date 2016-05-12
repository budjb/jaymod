#include <bgame/impl.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

void
__fastLerp( const vec3_t start, const vec3_t end, float fraction, vec3_t out )
{
    out[0] = start[0] + ((end[0] - start[0]) * fraction);
    out[1] = start[1] + ((end[1] - start[1]) * fraction);
    out[2] = start[2] + ((end[2] - start[2]) * fraction);
}

///////////////////////////////////////////////////////////////////////////////

} // namepsace anonymous

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume::AbstractHitVolume(
    type_t            type_,
    zone_t            zone_,
    entityType_t      entityType_,
    AbstractHitModel& hitModel_,
    scope_t           scope_ )

    : _hitModel  ( hitModel_ )
    , _entity    ( 0 )
    , _flags     ( HVF_ENABLED )
    , type       ( type_ )
    , scope      ( scope_ )
    , zone       ( zone_ )
    , entityType ( entityType_ )
    , flags      ( _flags )
{
    if (scope == SCOPE_PLAYER)
        _hitModel.registerHitVolume( *this );
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume::~AbstractHitVolume()
{
    entityFree();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::entityAlloc()
{
    if (_entity)
        return;

    _entity = G_Spawn();

    _entity->s.eType       = entityType;
    _entity->s.clientNum   = _hitModel.client.slot;
    _entity->s.modelindex  = _hitModel.vitality | (scope == SCOPE_PLAYER ? 0x80 : 0);
    _entity->s.eFlags      = 0;
    _entity->s.effect1Time = 0;
    _entity->parent        = &_hitModel.client.gentity;

    if (cvars::g_hitmodeDebug.ivalue & AbstractHitModel::DEBUG_LIFECYCLE)
        _hitModel.debug << "entityAlloc: " << _entity->s.number << endl;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::entityCompute()
{
    if (!_entity)
        return;

    _entity->s.eFlags = _flags;
    doEntityCompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::entityFree()
{
    if (!_entity)
        return;

    if (cvars::g_hitmodeDebug.ivalue & AbstractHitModel::DEBUG_LIFECYCLE)
        _hitModel.debug << "entityFree: " << _entity->s.number << endl;

    G_FreeEntity( _entity );
    _entity = 0;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::expand( const AbstractHitVolume& vol )
{
    if (vol.mins[0] < mins[0])
        mins[0] = vol.mins[0];

    if (vol.mins[1] < mins[1])
        mins[1] = vol.mins[1];

    if (vol.mins[2] < mins[2])
        mins[2] = vol.mins[2];

    if (vol.maxs[0] > maxs[0])
        maxs[0] = vol.maxs[0];

    if (vol.maxs[1] > maxs[1])
        maxs[1] = vol.maxs[1];

    if (vol.maxs[2] > maxs[2])
        maxs[2] = vol.maxs[2];
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::lerp( AbstractHitVolume& end, float fraction )
{
    __fastLerp( mins, end.mins, fraction, mins );
    __fastLerp( maxs, end.maxs, fraction, maxs );
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume&
AbstractHitVolume::operator=( const AbstractHitVolume& obj )
{
    _flags = obj._flags;
    VectorCopy( obj.mins, mins );
    VectorCopy( obj.maxs, maxs );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::recordHit()
{
    if (!_entity)
        return;

    _entity->s.effect1Time++;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::set( hitVolumeFlags_t f, bool on )
{
    if (on)
        _flags |= f;
    else
        _flags &= ~f;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::set( hitVolumeFlags_t f )
{
    _flags |= f;
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractHitVolume::toString( type_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitVolume::toString( type_t value, string& out )
{
    switch (value) {
        case TYPE_ALIGNED_CUBOID:  out = "ALIGNED_CUBOID";  return out;
        case TYPE_ORIENTED_CUBOID: out = "ORIENTED_CUBOID"; return out;
        case TYPE_SPHERE:          out = "TYPE_SPHERE";     return out;
        case TYPE_CYLINDER:        out = "TYPE_CYLINDER";   return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractHitVolume::toString( scope_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitVolume::toString( scope_t value, string& out )
{
    switch (value) {
        case SCOPE_WORLD:  out = "WORLD";  return out;
        case SCOPE_PLAYER: out = "PLAYER"; return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractHitVolume::toString( zone_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitVolume::toString( zone_t value, string& out )
{
    switch (value) {
        case ZONE_BODY:        out = "BODY";        return out;
        case ZONE_HEAD:        out = "HEAD";        return out;
        case ZONE_ARM_LEFT:    out = "ARM_LEFT";    return out;
        case ZONE_ARM_RIGHT:   out = "ARM_RIGHT";   return out;
        case ZONE_HAND_LEFT:   out = "HAND_LEFT";   return out;
        case ZONE_HAND_RIGHT:  out = "HAND_RIGHT";  return out;
        case ZONE_TORSO:       out = "TORSO";       return out;
        case ZONE_TORSO_LEFT:  out = "TORSO_LEFT";  return out;
        case ZONE_TORSO_RIGHT: out = "TORSO_RIGHT"; return out;
        case ZONE_LEGS:        out = "LEGS";        return out;
        case ZONE_LEG_LEFT:    out = "LEG_LEFT";    return out;
        case ZONE_LEG_RIGHT:   out = "LEG_RIGHT";   return out;
        case ZONE_FOOT_LEFT:   out = "FOOT_LEFT";   return out;
        case ZONE_FOOT_RIGHT:  out = "FOOT_RIGHT";  return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitVolume::unset( hitVolumeFlags_t f )
{
    _flags &= ~f;
}
