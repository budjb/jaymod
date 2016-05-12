#ifndef GAME_CMD_ABSTRACTCOMMAND_H
#define GAME_CMD_ABSTRACTCOMMAND_H

///////////////////////////////////////////////////////////////////////////////

class AbstractCommand
{
public:
    enum PostAction {
        PA_NONE,
        PA_USAGE,
        PA_ERROR,
        _PA_MAX,
        _PA_UNDEFINED,
    };

    typedef vector<string> Args;

public:
    class Context
    {
    public:
        Context( Client*, bool = false );
        ~Context();

        Buffer        _ebuf;    // error buffer if needed
        Client* const _client;  // null when server-console or client
        User&         _user;    // User::CONSOLE or client's user
        Args          _args;    // command-line arguments
        const bool    _silent;  // silent command processing is desired
    };

protected:
    AbstractCommand( Privilege::Type, const char*, bool );

    virtual PostAction doExecute( Context& ) = 0;

    Buffer __usage;
    Buffer __descr;

public:
    virtual ~AbstractCommand();

    bool execute( Context& );
    bool hasPermission( const Context& );

    const Privilege _privilege;

    const string  _name;
    const Buffer& _usage;
    const Buffer& _descr;

private:
    static bool lookupPLAYER( const string&, vector<Client*>&, string& );

protected:
    static bool isAlive            ( const Client& );
    static bool isAliveError       ( const Client&, Context& );
    static bool isBot              ( const Client& );
    static bool isBotError         ( const Client&, Context& );
    static bool isDead             ( const Client& );
    static bool isDeadError        ( const Client&, Context& );
    static bool isHigherLevel      ( const Client&, const Client* );
    static bool isHigherLevelError ( const Client&, Context& );
    static bool isHigherLevelError ( const User&, Context& );
    static bool isNotOnTeam        ( const Client& );
    static bool isNotOnTeamError   ( const Client&, Context& );
    static bool isPlayingDead      ( const Client& );
    static bool isPlayingDeadError ( const Client&, Context& );

    static Level& lookupLEVEL  ( const string&, Context&, string = "LEVEL" );
    static bool   lookupPLAYER ( const string&, Context&, Client*&, string = "PLAYER" );
    static bool   lookupPLAYER ( const string&, Context&, vector<Client*>&, string = "PLAYER" );
    static User&  lookupUSER   ( const string&, Context&, string = "USER" );

public:
    static InlineText _ovalue;
};

///////////////////////////////////////////////////////////////////////////////

#include <game/cmd/AbstractBuiltin.h>

///////////////////////////////////////////////////////////////////////////////

typedef map<string,AbstractCommand*> Registry;

extern Registry registry;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_ABSTRACTCOMMAND_H
