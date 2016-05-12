#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

Privilege::Privilege( Type type, const string& name, bool grantAlways )
    : _type        ( type )
    , _grantAlways ( grantAlways )
    , _name        ( toName( type, name ))
    , _nameLower   ( str::toLowerCopy( _name ))
{
    PrivilegeSet::REGISTRY.insert( *this );
}

///////////////////////////////////////////////////////////////////////////////

Privilege::~Privilege()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
Privilege::operator==( const Privilege& ref ) const
{
    return this == &ref;
}

///////////////////////////////////////////////////////////////////////////////

bool
Privilege::operator!=( const Privilege& ref ) const
{
    return this != &ref;
}

///////////////////////////////////////////////////////////////////////////////

string
Privilege::toName( Type type, const string& name )
{
    static const string prefixPseudo     = "@";
    static const string prefixBehavioral = "B/";
    static const string prefixCommand    = "C/";
    static const string prefixCustom     = "X/";
    static const string prefixUndefined  = "?";

    switch (type) {
        case TYPE_PSEUDO:      return (prefixPseudo + name);
        case TYPE_BEHAVIORAL:  return (prefixBehavioral + name);
        case TYPE_COMMAND:     return (prefixCommand + name);
        case TYPE_CUSTOM:      return (prefixCustom + name);

        default:
            return (prefixUndefined + name);
    }
}
