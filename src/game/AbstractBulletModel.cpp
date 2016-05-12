#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

AbstractBulletModel::AbstractBulletModel( type_t type_, Client& client_, bool principal_ )
    : _visible       ( false )
    , _reference     ( 0 )
    , type           ( type_ )
    , principal      ( principal_ )
    , debug          ( string("bulletModel[") + toString(type_) + ","
                           + (principal_ ? "PRINCIPAL" : "REFERENCE") + "]", client_.debug )
    , client         ( client_ )
    , visible        ( _visible )
{
    if (cvars::g_bulletmodeDebug.ivalue & DEBUG_LIFECYCLE)
        debug << "constructed: " << this << endl;
}

///////////////////////////////////////////////////////////////////////////////

AbstractBulletModel::~AbstractBulletModel()
{
    delete _reference;

    if (cvars::g_bulletmodeDebug.ivalue & DEBUG_LIFECYCLE)
        debug << "destroyed: " << this << endl;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletModel::cvarTrail( Cvar& var )
{
    if (var.ivalue < 0)
        var.set( 0 );
    else if (var.ivalue > 25)
        var.set( 25 );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractBulletModel::factory( AbstractBulletModel*& result, Client& client, type_t type, bool principal )
{
    switch (type) {
        case AbstractBulletModel::TYPE_ENTITY:
            result = new EntityBulletModel( client, principal );
            break;

        case AbstractBulletModel::TYPE_ETMAIN:
            result = new EtmainBulletModel( client, principal );
            break;

        case AbstractBulletModel::TYPE_MUZZLE:
            result = new MuzzleBulletModel( client, principal );
            break;

        default:
            result = new EtmainBulletModel( client, principal );
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletModel::firePlayer( TraceContext& trx )
{
    const list<AbstractBulletVolume*>::iterator end = _bulletVolumeList.end();
    for ( list<AbstractBulletVolume*>::iterator it = _bulletVolumeList.begin(); it != end; it++ )
        (*it)->fire( trx );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractBulletModel::fireWorld( TraceContext& trx )
{
    // TODO: nuke when done with scale testing
    if (cvars::g_test.ivalue & G_TEST_SKIP_FIRE)
        return false;

    bool hit = fireWorldAtomic( trx, false );

    if (trx.client && trx.client->bulletModel->_reference) {
        TraceContext trx2 = trx;
        fireWorldAtomic( trx2, true );
    }

    return hit;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractBulletModel::fireWorldAtomic( TraceContext& trx, bool ref )
{
    using namespace text;

    // Save debug size so we can discard if empty.
    const uint32 savedDebugLength = trx.debug.length;

    if (cvars::g_bulletmodeDebug.ivalue & DEBUG_TFIRE) {
        InlineText colA;

        colA.flags |= ios::left;
        colA.width  = 13;
        colA.suffix = " = ";

        trx.debug << xheaderBOLD( JAYMOD_FUNCTION ) << xlindent
            << '\n' << colA( "source"        ) << xvalue ( trx.source.s.number )
            << '\n' << colA( "source.origin" ) << xvec3  ( trx.source.r.currentOrigin )
            << '\n' << colA( "actor"         ) << xvalue ( trx.actor.s.number )
            << '\n' << colA( "actor.origin"  ) << xvec3  ( trx.actor.r.currentOrigin )
            << '\n' << colA( "content mask"  ) << xvalue ( str::toHexString( trx.mask ))
            << '\n' << colA( "start"         ) << xvec3  ( trx.start )
            << '\n' << colA( "end"           ) << xvec3  ( trx.end )
            << xlunindent << '\n';
    }

    bool hit = AbstractHitModel::traceWorld( trx );

    if (trx.client) {
        if (ref)
            trx.client->bulletModel->_reference->firePlayer( trx );
        else
            trx.client->bulletModel->firePlayer( trx );

        if (trx.debug.length > savedDebugLength)
            trx.client->xprint( trx.debug );
    }

    return hit;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletModel::registerBulletVolume( AbstractBulletVolume& bv )
{
    _bulletVolumeList.push_back( &bv );
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletModel::run()
{
    updateVisibility();

    const list<AbstractBulletVolume*>::iterator end = _bulletVolumeList.end();
    for ( list<AbstractBulletVolume*>::iterator it = _bulletVolumeList.begin(); it != end; it++ )
        (*it)->entityCompute();

    ///////////////////////////////////////////////////////////////
    //
    // reference model section
    //
    ///////////////////////////////////////////////////////////////

    if (!principal)
        return;

    if (cvars::g_bulletmodeDebug.ivalue & DEBUG_RDRAW) {
        bool construct = false;
        if (_reference) {
            if (cvars::g_bulletmodeReference.ivalue != _reference->type) {
                delete _reference;
                construct = true;
            }
        }
        else {
            construct = true;
        } 
    
        if (construct) {
            if ( AbstractBulletModel::factory(
                    _reference,
                    client,
                    static_cast< AbstractBulletModel::type_t >( cvars::g_bulletmodeReference.ivalue ),
                    false ))
            {
                // Cvar value must be invalid but factory gave us default implementation,
                // thus cvar should be updated to reflect active type.
                cvars::g_bulletmodeReference.set( _reference->type );
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

string
AbstractBulletModel::toString( dflags_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractBulletModel::toString( dflags_t value, string& out )
{
    out = "";
    ostringstream oss;
    for (int i = 0; i < 32; i++) {
        if (i > 0)
            out += ',';

        switch (1 << i) {
            case DEBUG_BDRAW:     out += "BDRAW";     break;
            case DEBUG_RDRAW:     out += "RDRAW";     break;
            case DEBUG_LIFECYCLE: out += "LIFECYCLE"; break;
            case DEBUG_TFIRE:     out += "TFIRE";     break;

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
AbstractBulletModel::toString( type_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractBulletModel::toString( type_t value, string& out )
{
    switch (value) {
        case TYPE_ENTITY: out = "ENTITY"; return out;
        case TYPE_ETMAIN: out = "ETMAIN"; return out;
        case TYPE_MUZZLE: out = "MUZZLE"; return out;

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
AbstractBulletModel::updateVisibility()
{
    if (cvars::g_bulletmodeDebug.ivalue) {
        if (_visible)
            return;

        _visible = true;

        const list<AbstractBulletVolume*>::iterator end = _bulletVolumeList.end();
        for ( list<AbstractBulletVolume*>::iterator it = _bulletVolumeList.begin(); it != end; it++ )
            (*it)->entityAlloc();
    }
    else {
        if (!_visible)
            return;

        _visible = false;

        const list<AbstractBulletVolume*>::reverse_iterator end = _bulletVolumeList.rend();
        for ( list<AbstractBulletVolume*>::reverse_iterator it = _bulletVolumeList.rbegin(); it != end; it++ )
            (*it)->entityFree();
    }
}
