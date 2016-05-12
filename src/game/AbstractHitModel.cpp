#include <bgame/impl.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

static list<AbstractHitModel*> __ghosts;

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

AbstractHitModel::AbstractHitModel( type_t type_, Client& client_, vitality_t vitality_ )
    : _visible       ( false )
    , _reference     ( 0 )
    , _time          ( -1 )
    , _hitVolumeList ( )
    , type           ( type_ )
    , vitality       ( vitality_ )
    , debug          ( string("hitModel[") + toString(type_) + "," + toString(vitality_) + "]", client_.debug )
    , client         ( client_ )
    , visible        ( _visible )
    , time           ( _time )
    , worldVol       ( AbstractHitVolume::_ZONE_UNDEFINED, *this, AbstractHitVolume::SCOPE_WORLD )
{
    if (cvars::g_hitmodeDebug.ivalue & DEBUG_LIFECYCLE) {
        if (vitality == VITALITY_GHOST) {
            if (cvars::g_hitmodeDebug.ivalue & DEBUG_SNAPSHOT)
                debug << "constructed:" << this << endl;
        }
        else {
            debug << "constructed:" << this << endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitModel::~AbstractHitModel()
{
    const list<AbstractHitModel*>::iterator end = _snapshots.end();
    for ( list<AbstractHitModel*>::iterator it = _snapshots.begin(); it != end; it++ )
        delete *it;

    delete _reference;

    if (cvars::g_hitmodeDebug.ivalue & DEBUG_LIFECYCLE) {
        if (vitality == VITALITY_GHOST) {
            if (cvars::g_hitmodeDebug.ivalue & DEBUG_SNAPSHOT)
                debug << "destroyed:" << this << endl;
        }
        else {
            debug << "destroyed:" << this << endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::cvarAntilag( Cvar& var )
{
    if (var.ivalue < 0)
        var.set( 0 );
    else if (var.ivalue > 1500)
        var.set( 1500 );

    for (int i = 0; i < MAX_CLIENTS; i++)
        g_clientObjects[i].hitModel->snapshotPrune();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::cvarAntilagLerp( Cvar& var )
{
    if (var.ivalue < 0)
        var.set( 0 );
    else if (var.ivalue > 1)
        var.set( 1 );
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::cvarFat( Cvar& var )
{
    if (var.fvalue < -10.0f)
        var.set( -10.0f );
    else if (var.fvalue > 10.0f)
        var.set( 10.0f );
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::cvarGhosting( Cvar& var ) 
{
    if (var.ivalue < 0)
        var.set( 0 );
    else if (var.ivalue > 30000)
        var.set( 30000 );

    ghostPrune();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::cvarZone( Cvar& var )
{
    if (var.ivalue < AbstractHitVolume::_ZONE_UNDEFINED || var.ivalue >= AbstractHitVolume::_ZONE_MAX)
        var.set( AbstractHitVolume::_ZONE_UNDEFINED );
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume*
AbstractHitModel::doTracePlayer( TraceContext& trx )
{
    AbstractHitVolume* result = 0;

    float  hshortest = FLT_MAX;
    vec3_t hpos;
    float  hlen;

    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (!(hv.flags & HVF_ENABLED))
            continue;

        if (!hv.castRay( trx, hpos, hlen ))
            continue;

        if (hlen < hshortest) {
            hshortest = hlen;
            result = &hv;

            // Update final pos/len.
            VectorCopy( hpos, trx.fpos );
            trx.flen = hlen;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractHitModel::factory( AbstractHitModel*& result, Client& client, type_t type, vitality_t vitality )
{
    switch (type) {
        case AbstractHitModel::TYPE_ENTITY:
            result = new EntityHitModel( client, vitality );
            break;

        case AbstractHitModel::TYPE_ETMAIN:
            result = new EtmainHitModel( client, vitality );
            break;

        case AbstractHitModel::TYPE_BASIC:
            result = new BasicHitModel( client, vitality );
            break;

        case AbstractHitModel::TYPE_STANDARD:
            result = new StandardHitModel( client, vitality );
            break;

        case AbstractHitModel::TYPE_ADVANCED:
            result = new AdvancedHitModel( client, vitality );
            break;

        case AbstractHitModel::TYPE_ORIENTED:
            result = new OrientedHitModel( client, vitality );
            break;

        default:
            result = new StandardHitModel( client, vitality );
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::ghostCleanup()
{
    const list<AbstractHitModel*>::iterator end = __ghosts.end();
    for ( list<AbstractHitModel*>::iterator it = __ghosts.begin(); it != end; it++ )
        delete *it;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::ghostPrune()
{
    const int cutoff = level.time - cvars::g_hitmodeGhosting.ivalue;

    while (!__ghosts.empty()) {
        AbstractHitModel& hm = *__ghosts.back();
        if (hm._time >= cutoff)
            break;

        __ghosts.pop_back();
        delete &hm;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::lerp( AbstractHitModel& hmend, float fraction )
{
    worldVol.lerp( hmend.worldVol, fraction );

    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (!(hv.flags & HVF_ENABLED))
            continue;

        AbstractHitVolume* e = hmend.volumeForZone( hv.zone );
        if (!e)
            continue;

        hv.lerp( *e, fraction );
    }
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitModel&
AbstractHitModel::operator=( const AbstractHitModel& obj )
{
    _visible = obj._visible;
    _time    = obj._time;
    worldVol = obj.worldVol;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::registerHitVolume( AbstractHitVolume& hv )
{
    _hitVolumeList.push_back( &hv );
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::recordHit( AbstractHitVolume::zone_t z )
{
    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (hv.zone != z)
            continue;

        hv.recordHit();
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::run()
{
    snapshotPrune();
    snapshot();  // must be before _time is set

    _time = level.time;

    updateVisibility();  // must be called before doRun()
    doRun();

    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin();
    for ( ; it != end; it++ )
        (*it)->entityCompute();

    ///////////////////////////////////////////////////////////////
    //
    // worldVol section
    //
    ///////////////////////////////////////////////////////////////

    // Update worldVol bounds.
    // We assume there will always be at least one managed hit-volume.
    it = _hitVolumeList.begin();
    VectorCopy( (*it)->mins, worldVol.mins );
    VectorCopy( (*it)->maxs, worldVol.maxs );

    for ( it++; it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (!(hv.flags & HVF_ENABLED))
            continue;

        worldVol.expand( hv );
    }

    worldVol.entityCompute();

    ///////////////////////////////////////////////////////////////
    //
    // reference model section
    //
    ///////////////////////////////////////////////////////////////

    if (vitality != VITALITY_PRINCIPAL)
        return;

    if (cvars::g_hitmodeDebug.ivalue & DEBUG_RDRAW) {
        bool construct = false;
        if (_reference) {
            if (cvars::g_hitmodeReference.ivalue != _reference->type) {
                delete _reference;
                construct = true;
            }
        }
        else {
            construct = true;
        }

        if (construct) {
            if ( AbstractHitModel::factory(
                    _reference,
                    client,
                    static_cast< AbstractHitModel::type_t >( cvars::g_hitmodeReference.ivalue ),
                    AbstractHitModel::VITALITY_REFERENCE ))
            {
                // Cvar value must be invalid but factory gave us default implementation,
                // thus cvar should be updated to reflect active type.
                cvars::g_hitmodeReference.set( _reference->type );
            }
        }
    }
    else {
        if (_reference) {
            delete _reference;
            _reference = 0;
        }
    }

    if (_reference)
        _reference->run();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::snapshot()
{
    // Skip snapshot attempt before first run.
    if (_time == -1)
        return;

    if (cvars::g_hitmodeAntilag.ivalue < 1)
        return;

    _snapshots.push_front( doSnapshot() );
    snapshotPrune();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::snapshotPrune()
{
    const int cutoff = level.time - cvars::g_hitmodeAntilag.ivalue;

    while (!_snapshots.empty()) {
        AbstractHitModel& hm = *_snapshots.back();
        if (hm._time >= cutoff)
            break;

        _snapshots.pop_back();
        delete &hm;
    }
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractHitModel::toString( dflags_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitModel::toString( dflags_t value, string& out )
{
    out = "";
    ostringstream oss;
    for (int i = 0; i < 32; i++) {
        if (i > 0)
            out += ',';

        switch (1 << i) {
            case DEBUG_HDRAW:     out += "HDRAW";     break;
            case DEBUG_RDRAW:     out += "RDRAW";     break;
            case DEBUG_WDRAW:     out += "WDRAW";     break;
            case DEBUG_LIFECYCLE: out += "LIFECYCLE"; break;
            case DEBUG_SNAPSHOT:  out += "SNAPSHOT";  break;
            case DEBUG_STATE:     out += "STATE";     break;
            case DEBUG_TRAY:      out += "TRAY";      break;
            case DEBUG_TVOLUME:   out += "TVOLUME";   break;

            default:
                oss.str( "" );
                oss << "UNDEFINED(" << value << ")";
                out += oss.str();
                break;
        }
    }

    return out;
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractHitModel::toString( type_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitModel::toString( type_t value, string& out )
{
    switch (value) {
        case TYPE_ENTITY:    out = "ENTITY";    return out;
        case TYPE_ETMAIN:    out = "ETMAIN";    return out;
        case TYPE_BASIC:     out = "BASIC";     return out;
        case TYPE_STANDARD:  out = "STANDARD";  return out;
        case TYPE_ADVANCED:  out = "ADVANCED";  return out;
        case TYPE_ORIENTED:  out = "ORIENTED";  return out;

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
AbstractHitModel::toString( vitality_t value )
{
    string result;
    return toString( value, result );
}   

///////////////////////////////////////////////////////////////////////////////

string&
AbstractHitModel::toString( vitality_t value, string& out )
{
    switch (value) {
        case VITALITY_PRINCIPAL: out = "PRINCIPAL"; return out;
        case VITALITY_REFERENCE: out = "REFERENCE"; return out;
        case VITALITY_GHOST:     out = "GHOST";     return out;

        default:
            break;
    }
    
    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}   

///////////////////////////////////////////////////////////////////////////////

bool
AbstractHitModel::tracePlayer( TraceContext& trx )
{
    trx.hitvol = doTracePlayer( trx );
    if (!trx.hitvol)
        return false;

    switch (vitality) {
        case VITALITY_GHOST:
            // Record hit on PRINCIPAL hitmodel too.
            client.hitModel->recordHit( trx.hitvol->zone );

            if (cvars::g_hitmodeGhosting.ivalue) {
                // Visualize entities.
                // Set bit on all hit-vols which marks the box as hit without flash.
                const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
                for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
                    AbstractHitVolume& hv = **it;

                    hv.set( HVF_HIT, hv.zone == trx.hitvol->zone );
                    hv.set( HVF_DRAW );
                    hv.entityAlloc();
                    hv.entityCompute();
                }
            }
            break;

        default:
            trx.hitvol->recordHit();
            break;
    }

    if (cvars::g_hitmodeDebug.ivalue & DEBUG_TRAY) {
        using namespace text;

        InlineText colA;

        colA.flags |= ios::left;
        colA.width = 14;
        colA.suffixOutside = " = ";

        trx.debug << xheader( JAYMOD_FUNCTION ) << xlindent
            << "\n" << colA( "hit"     ) << xvalue( true )
            << "\n" << colA( "hv"      ) << xvalue( str::toString( trx.hitvol ))
            << "\n" << colA( "hv.type" ) << xvalue( AbstractHitVolume::toString( trx.hitvol->type ))
            << "\n" << colA( "hv.zone" ) << xvalue( AbstractHitVolume::toString( trx.hitvol->zone ))
            << xlunindent << "\n";
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::tracePlayerBegin( TraceContext& trx )
{
    using namespace text;

    InlineText colA;
    InlineText colB = xvalue;

    const bool dbg = cvars::g_hitmodeDebug.ivalue & DEBUG_SNAPSHOT;
    if (dbg) {
        colA.flags |= ios::left;
        colA.width = 15;
        colA.suffixOutside  = " = ";

        colB.width = 7;

        trx.debug << xheaderBOLD( JAYMOD_FUNCTION ) << xlindent
            << "\n" << colA( "trx.time" )    << colB( trx.time )
                    << " (" << xvalue( level.time - trx.time ) << " delta" << ")"
            << "\n" << colA( "client.slot" ) << colB( client.slot )
                    << " (" << xvalue(_snapshots.size()) << " frames" << ")"
            << "\n" << colA( "level.time" )  << colB( level.time );
    }

    if (cvars::g_hitmodeAntilag.ivalue && trx.time < level.time) {
        AbstractHitModel* hmUpper = _contextHitModel;
        AbstractHitModel* hmLower = _contextHitModel;

        int fi = 0; 
        const list<AbstractHitModel*>::iterator end = _snapshots.end();
        for ( list<AbstractHitModel*>::iterator it = _snapshots.begin(); it != end; it++, fi++ ) {
            AbstractHitModel& hm = **it;

            if (dbg)
               trx.debug << "\n" << colA( fi ) << colB( hm._time );

            if (trx.time < hm._time) { 
                hmUpper = &hm;
                continue;
            }

            hmLower = &hm;
            break;
        }

        if (hmLower->_time > hmUpper->_time)
            hmLower = hmUpper;

        if (dbg) {
            trx.debug << "\n" << colA( "upper.frameTime" ) << colB( hmUpper->_time )
                      << "\n" << colA( "lower.frameTime" ) << colB( hmLower->_time );
        }

        if (hmLower->_time == hmUpper->_time) {
            if (_time != hmUpper->_time) {
                // We have exact snapshot, no lerp required.
                _contextHitModel = hmUpper;

                if (dbg)
                    trx.debug << "\n" << colA( "snap.exact" ) << colB( _contextHitModel->_time );
            }
            else {
                // We will be using the most recent frame.
                if (dbg)
                    trx.debug << "\n" << colA( "snap.none" ) << colB( _contextHitModel->_time );
            }
        }
        else if (cvars::g_hitmodeAntilagLerp.ivalue) {
            // LERP enabled.
            _contextHitModel = hmLower->doSnapshot();
            __ghosts.push_front( _contextHitModel );
            _contextHitModel->lerp( *hmUpper,
                float(trx.time - hmLower->_time) / float(hmUpper->_time - hmLower->_time) );

            if (dbg) {
                trx.debug << "\n" << colA( "snap.lerp" )
                                  << colB( _contextHitModel->_time ) << " --> " << colB( hmUpper->_time );
            }
        }
        else {
            // LERP disabled.
            _contextHitModel = hmUpper;

            if (dbg)
                trx.debug << "\n" << colA( "snap.exact" ) << colB( _contextHitModel->_time );
        }
    }

    VectorCopy( client.gentity.r.mins, _originalBounds[0] );
    VectorCopy( client.gentity.r.maxs, _originalBounds[1] );

    VectorCopy( _contextHitModel->worldVol.mins, client.gentity.r.mins );
    VectorSubtract( client.gentity.r.mins, client.gentity.r.currentOrigin, client.gentity.r.mins );

    VectorCopy( _contextHitModel->worldVol.maxs, client.gentity.r.maxs );
    VectorSubtract( client.gentity.r.maxs, client.gentity.r.currentOrigin, client.gentity.r.maxs );

if (!(cvars::g_test.ivalue & G_TEST_SKIP_LINK))
    trap_LinkEntity( &client.gentity );

    if (dbg) {
        trx.debug
            << "\n" << colA( "original.mins" ) << xvec3( _originalBounds[0] )
            << "\n" << colA( "original.maxs" ) << xvec3( _originalBounds[1] )
            << "\n" << colA( "r.mins" )        << xvec3( client.gentity.r.mins )
            << "\n" << colA( "r.maxs" )        << xvec3( client.gentity.r.maxs )
            << xlunindent << "\n";
    }
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::tracePlayerEnd( TraceContext& trx )
{
    VectorCopy( _originalBounds[0], client.gentity.r.mins );
    VectorCopy( _originalBounds[1], client.gentity.r.maxs );

if (!(cvars::g_test.ivalue & G_TEST_SKIP_LINK))
    trap_LinkEntity( &client.gentity );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractHitModel::traceWorld( TraceContext& trx )
{
    // List of flags indicating if client was reconciled.
    bitset<MAX_CLIENTS> reconciled;
    bitset<MAX_CLIENTS> unlinked;

    // Ready players in world for trace.
    const bool antilag = trx.client && cvars::g_hitmodeAntilag.ivalue && trx.client->gclient.pers.antilag;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Client& client = g_clientObjects[i];

        /* We must GUARANTEE that _contextHitModel is initialized to latest copy for all clients.
         * Then if all criteria is met, it may be replaced with historical snapshot.
         */
        client.hitModel->_contextHitModel = client.hitModel;

        if (!antilag)
            continue;

        // Skip self.
        if (trx.client->slot == client.slot)
            continue;

        if (!client.isReconcileSafe())
            continue;

        reconciled.set( client.slot );
        client.hitModel->tracePlayerBegin( trx );
    }

    // Remove self from world tracing to prevent shooting self.
    if (trx.client && trx.client->gentity.r.linked) {
        unlinked.set( trx.client->slot );
        trap_UnlinkEntity( &trx.client->gentity );
    }

    // Bullet might hit world-volume player, and then miss all sub-volumes.
    // Since this is very likely (eg. shoot between player's legs and hit player on other side)
    // it is important for us to continue the trace. Our strategy will be to unlink player,
    // and repeat. However, the theoretical limit of this is MAX_CLIENTS, and just to be
    // safe we'll limit the number of near-misses accordingly.

    bool hit = false;
    for (int wi = 0; wi < MAX_CLIENTS; wi++) {
        // Check for hit against aa-bbox (worldVol).
        hit = trx.trace( JAYMOD_FUNCTION, wi );

        // No need to continue if we scored no hit.
        if (!hit)
            break;

        // No need to continue if hit was not player.
        if (!trx.resultIsPlayer())
            break;

        // Perform sub-volumes hit tracing.
        Client& client = g_clientObjects[trx.data.entityNum];
        hit = client.hitModel->_contextHitModel->tracePlayer( trx );

        // Sub-volume hit means we do not need to handle near-miss situation.
        if (hit)
            break;

        // Bullet missed all sub-volumes so unlink if linked, and retry.
        if (client.gentity.r.linked) {
            unlinked.set( client.slot );
            trap_UnlinkEntity( &client.gentity );
        }
    }

    // Restore reconciled players in world after trace.
    if (antilag) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (reconciled[i])
                g_clientObjects[i].hitModel->tracePlayerEnd( trx );
        }
    }

    // Relink unlinked players.
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (unlinked[i])
            trap_LinkEntity( &g_clientObjects[i].gentity );
    }

    return hit;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractHitModel::updateVisibility()
{
    if (cvars::g_hitmodeDebug.ivalue & DEBUG_WDRAW)
        worldVol.set( HVF_DRAW );
    else
        worldVol.unset( HVF_DRAW );

    const int drawFlag = (vitality == VITALITY_PRINCIPAL) ? DEBUG_HDRAW : DEBUG_RDRAW;
    if ((cvars::g_hitmodeDebug.ivalue & drawFlag) && client.isVisibleTarget()) {
        if (_visible)
            return;

        _visible = true;

        worldVol.entityAlloc();

        const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
        for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
            AbstractHitVolume& hv = **it;
            hv.entityAlloc();
            hv.set( HVF_DRAW );
        }
    }
    else {
        if (!_visible)
            return;

        _visible = false;

        const list<AbstractHitVolume*>::reverse_iterator end = _hitVolumeList.rend();
        for ( list<AbstractHitVolume*>::reverse_iterator it = _hitVolumeList.rbegin(); it != end; it++ ) {
            AbstractHitVolume& hv = **it;

            hv.entityFree();
            hv.unset( HVF_DRAW );
        }

        worldVol.entityFree();
    }
}

///////////////////////////////////////////////////////////////////////////////

AbstractHitVolume*
AbstractHitModel::volumeForZone( AbstractHitVolume::zone_t z )
{
    const list<AbstractHitVolume*>::iterator end = _hitVolumeList.end();
    for ( list<AbstractHitVolume*>::iterator it = _hitVolumeList.begin(); it != end; it++ ) {
        AbstractHitVolume& hv = **it;
        if (hv.zone == z)
            return &hv;
    }

    return 0;
}
