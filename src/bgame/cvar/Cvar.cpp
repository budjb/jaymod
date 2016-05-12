#include <bgame/impl.h>

namespace cvar {

///////////////////////////////////////////////////////////////////////////////

Cvar::Cvar( const string& name_, const string& defaultValue_, int flags_, void(*callback_)(Cvar&), bool update )
    : _callback                  ( callback_ )
    , _callbackModificationCount ( -1 )
    , name                       ( name_ )
    , defaultValue               ( defaultValue_ )
    , flags                      ( flags_ )
    , handle                     ( _data.handle )
    , modificationCount          ( _data.modificationCount )
    , fvalue                     ( _data.value )
    , ivalue                     ( _data.integer )
    , svalue                     ( _data.string )
    , lastModificationCount      ( -1 )
{
    REGISTRY.push_back( this );

    if (update)
        UPDATE.push_back( this );
}

///////////////////////////////////////////////////////////////////////////////

Cvar::~Cvar()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::init()
{
    const list<Cvar*>::iterator end = Cvar::REGISTRY.end();
    for ( list<Cvar*>::iterator it = Cvar::REGISTRY.begin(); it != end; it++ ) {
        Cvar& v = **it;
        v.trapRegister();

        // cvar has indicated it wants an init-time callback
        if (v.flags & CVAR_JAYMODCB_INIT)
            v._callback( v );
    }
}

///////////////////////////////////////////////////////////////////////////////

const list<Cvar*>&
Cvar::getUpdateList()
{
    return UPDATE;
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::set( float value )
{
    ostringstream oss;
    oss << value;
    trap_Cvar_Set( name.c_str(), oss.str().c_str() );
    trapUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::set( int value )
{
    ostringstream oss;
    oss << value;
    trap_Cvar_Set( name.c_str(), oss.str().c_str() );
    trapUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::set( const string& value )
{
    trap_Cvar_Set( name.c_str(), value.c_str() );
    trapUpdate();
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::trapRegister()
{
    trap_Cvar_Register( &_data, name.c_str(), defaultValue.c_str(), flags );
}

///////////////////////////////////////////////////////////////////////////////

void
Cvar::trapUpdate()
{
    trap_Cvar_Update( &_data );

    if (!_callback)
        return;

    if (_callbackModificationCount == modificationCount)
        return;

    // To prevent an cicular dependency we temporarily remove callback.
    void(*tmp)(Cvar&) = _callback;
    _callback = 0;

    tmp( *this );
    _callbackModificationCount = modificationCount;

    // Restore callback.
    _callback = tmp;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cvar
