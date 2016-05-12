#ifndef GAME_PRIVILEGE_H
#define GAME_PRIVILEGE_H

///////////////////////////////////////////////////////////////////////////////

class Privilege
{
public:
    enum Type {
        TYPE_PSEUDO,      // special non-real types
        TYPE_BEHAVIORAL,  // non-commands
        TYPE_COMMAND,     // built-in commands
        TYPE_CUSTOM,      // custom commands
        _TYPE_MAX,
        _TYPE_UNDEFINED,
    };

    typedef const Privilege* Handle;

public:
    Privilege( Type, const string& name, bool = false );
    ~Privilege();

    bool operator==( const Privilege& ) const;
    bool operator!=( const Privilege& ) const;

    const Type   _type;
    const bool   _grantAlways;
    const string _name;
    const string _nameLower;

private:
    static string toName( Type, const string& );
};

///////////////////////////////////////////////////////////////////////////////

namespace priv {
    namespace pseudo {
        extern const Privilege
            all,
            behaviors,
            commands;
    } // namespace pseudo

    namespace base {
        extern const Privilege
            balanceImmunity,
            banPermanent,
            censorImmunity,
            commandChat,
            commandSilent,
            reasonNone,
            specChat,
            voteAny,
            voteImmunity;
    } // namespace base
} // namespace priv

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_PRIVILEGE_H
