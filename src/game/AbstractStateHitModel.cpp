#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

AbstractStateHitModel::AbstractStateHitModel( type_t type_, Client& client_, vitality_t vitality_ )
    : AbstractHitModel ( type_, client_, vitality_ )
    , _state           ( _STATE_UNDEFINED )
    , _headBox         ( AbstractHitVolume::ZONE_HEAD,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _footBox         ( AbstractHitVolume::ZONE_LEGS,  *this, AbstractHitVolume::SCOPE_PLAYER )
    , _torsoBox        ( AbstractHitVolume::ZONE_TORSO, *this, AbstractHitVolume::SCOPE_PLAYER )
    , state            ( _state )
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractStateHitModel::~AbstractStateHitModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractStateHitModel::doRun()
{
    state_t ns;
    const int eflags = client.gentity.s.eFlags;
    if (eflags & EF_CROUCHING)
        ns = STATE_CROUCH;
    else if (eflags & EF_DEAD)
        ns = STATE_DEAD;
    else if (eflags & EF_PLAYDEAD)
        ns = STATE_PLAYDEAD;
    else if (eflags & EF_PRONE)
        ns = STATE_PRONE;
    else
        ns = STATE_STAND;

    if (_state != ns) {
        if (cvars::g_hitmodeDebug.ivalue & DEBUG_STATE) {
            string buf;
            debug << toString( _state, buf ) << " -> ";
            debug << toString( ns, buf ) << endl;
        }

        _state = ns;
        doState();
    }

    doStateRun();
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume*
AbstractStateHitModel::doTracePlayer( TraceContext& trx )
{
    vec3_t hpos;
    float  hlen;

    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (!(hv.flags & HVF_ENABLED))
            continue;

        if (!hv.castRay( trx, hpos, hlen ))
            continue;

        // List is priority-based and with first hit we populate and return.
        // Update final pos/len.

        VectorCopy( hpos, trx.fpos );
        trx.flen = hlen;
        return &hv;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

AbstractStateHitModel&
AbstractStateHitModel::operator=( const AbstractStateHitModel& obj )
{
    AbstractHitModel::operator=( obj );

    _state    = obj._state;    
    _headBox  = obj._headBox;
    _footBox  = obj._footBox;
    _torsoBox = obj._torsoBox;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractStateHitModel::toString( state_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractStateHitModel::toString( state_t value, string& out )
{
    switch (value) {
        case STATE_CROUCH:    out = "CROUCH";   return out;
        case STATE_DEAD:      out = "DEAD";     return out;
        case STATE_PLAYDEAD:  out = "PLAYDEAD"; return out;
        case STATE_PRONE:     out = "PRONE";    return out;
        case STATE_STAND:     out = "STAND";    return out;

        default:
            break;
    }
    
    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}
