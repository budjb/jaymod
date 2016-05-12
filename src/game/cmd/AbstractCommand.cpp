#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::AbstractCommand( Privilege::Type privType, const char* name, bool grantAlways )
    : __usage    ( xcnormal )
    , __descr    ( xcnormal )
    , _privilege ( privType, name, grantAlways )
    , _name      ( name )
    , _usage     ( __usage )
    , _descr     ( __descr )
{
    registry[name] = this;
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::~AbstractCommand()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::execute( Context& txt )
{
    if (!hasPermission( txt )) {
        Buffer buf;
        buf << xfail( "Access denied: " ) << xvalue( _name );
        printChat( txt._client, buf );

        adminLog.log( txt._client, txt._args, true );
        return true;
    }

    adminLog.log( txt._client, txt._args, false );

    // prepare error buffer and prefix with command name
    txt._ebuf.reset();
    txt._ebuf << xfail( _name + " error: " );

    switch (doExecute( txt )) {
        default:
        case PA_NONE:
            return false;

        case PA_USAGE:
            break;

        case PA_ERROR:
            printChat( txt._client, txt._ebuf );
            return true;
    }

    Buffer buf;
    if (_usage.numLines == 1)
        buf << xfail( "usage: " ) << _usage;
    else
        buf << xfail( "usage:" ) << '\n' << xlindent << _usage;

    printChat( txt._client, buf );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::hasPermission( const Context& txt )
{
    return txt._user.hasPrivilege( _privilege );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isAlive( const Client& target )
{
    return (target.gentity.health > 0);
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isAliveError( const Client& target, Context& txt )
{
    if (target.gentity.health <= 0)
        return false;

    const User& targetUser = *connectedUsers[target.slot];
    txt._ebuf << xvalue( targetUser.namex) << " is alive.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isBot( const Client& target )
{
    return (target.gentity.r.svFlags & SVF_BOT);
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isBotError( const Client& target, Context& txt )
{
    if (!(target.gentity.r.svFlags & SVF_BOT))
        return false;

    const User& targetUser = *connectedUsers[target.slot];
    txt._ebuf << xvalue( targetUser.namex ) << " is a bot.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isDead( const Client& target )
{
    return (target.gentity.health <= 0);
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isDeadError( const Client& target, Context& txt )
{
    if (target.gentity.health > 0)
        return false;

    const User& targetUser = *connectedUsers[target.slot];
    txt._ebuf << xvalue( targetUser.namex) << " is dead.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isHigherLevel( const Client& target, const Client* actor )
{
    if (!actor)
        return false;

    return (connectedUsers[actor->slot]->authLevel < connectedUsers[target.slot]->authLevel );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isHigherLevelError( const Client& target, Context& txt )
{
    return isHigherLevelError( *connectedUsers[target.slot], txt );
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isHigherLevelError( const User& target, Context& txt )
{
    if (target.authLevel <= txt._user.authLevel)
        return false;

    txt._ebuf << xvalue( target.namex ) << " has the same or higher level than you.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isNotOnTeam( const Client& target )
{
    switch (target.gclient.sess.sessionTeam) {
        case TEAM_AXIS:
        case TEAM_ALLIES:
            return false;

        default:
            return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isNotOnTeamError( const Client& target, Context& txt )
{
    if (!isNotOnTeam( target ))
        return false;

    const User& targetUser = *connectedUsers[target.slot];
    txt._ebuf << xvalue( targetUser.namex ) << " is not on a team.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isPlayingDead( const Client& target )
{
    return (target.gclient.ps.eFlags & EF_PLAYDEAD);
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::isPlayingDeadError( const Client& target, Context& txt )
{
    if (!(target.gclient.ps.eFlags & EF_PLAYDEAD))
        return false;

    const User& targetUser = *connectedUsers[target.slot];
    txt._ebuf << xvalue( targetUser.namex) << " is playing dead.";
    return true;
}

///////////////////////////////////////////////////////////////////////////////

Level&
AbstractCommand::lookupLEVEL( const string& name, Context& txt, string argName )
{
    string err;
    Level& lev = levelDB.fetchByName( name, err );
    if (lev == Level::BAD)
        txt._ebuf << xvalue( argName ) << ' ' << err << '.';
    return lev;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::lookupPLAYER( const string& name, vector<Client*>& out, string& err )
{
    if (name.empty()) {
        err = "is empty";
        return true;
    }

    out.clear();

    // handle slot mode
    {
        bool slotmode = true;
        const string::size_type max = name.length();
        if (max > 2) {
            slotmode = false;
        }
        else {
            for ( string::size_type i = 0; i < max; i++ ) {
                const char c = name[i];
                if (c < '0' || c > '9') {
                    slotmode = false;
                    break;
                }
            }
        }

        if (slotmode) {
            const int slot = atoi( name.c_str() );
            if (slot < 0) {
                err = "minimum is 0";
                return true;
            }
            else if (slot >= MAX_CLIENTS) {
                err = "maximum is 63";
                return true;
            }

            Client& client = g_clientObjects[slot];
            if (client.gclient.pers.connected != CON_CONNECTED) {
                err = "not connected";
                return true;
            }

            out.push_back( &client );
            return false;
        }
    }

    string lname = name;
    str::toLower( lname );

    // search connected users
    for (int i = 0; i < MAX_CLIENTS; i++) {
        Client& client = g_clientObjects[i];
        if (client.gclient.pers.connected != CON_CONNECTED)
            continue;

        string cname = connectedUsers[i]->name;
        str::toLower( cname );
        if (cname.find( lname ) != string::npos)
            out.push_back( &client );
    }

    if (out.empty()) {
        err = "not found";
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::lookupPLAYER( const string& name, Context& txt, Client*& out, string argName )
{
    vector<Client*> clients;
    string err;
    if (lookupPLAYER( name, clients, err )) {
        txt._ebuf << xvalue( argName )  << ' ' << err << '.';
        return true;
    }

    if (clients.size() > 1) {
        const string mword = (clients.size() == 1 ? "match" : "matches");
        txt._ebuf << xvalue( argName ) << " is ambiguous (" << xvalue( clients.size() ) << ' ' << mword << ").";
        return true;
    }

    out = clients[0];
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractCommand::lookupPLAYER( const string& name, Context& txt, vector<Client*>& out, string argName )
{
    string err;
    if (!lookupPLAYER( name, out, err ))
        return false;

    txt._ebuf << xvalue( argName )  << ' ' << err << '.';
    return true;
}

///////////////////////////////////////////////////////////////////////////////

User&
AbstractCommand::lookupUSER( const string& id, Context& txt, string argName )
{
    string err;
    User& user = userDB.fetchByID( id, err );
    if (user == User::BAD)
        txt._ebuf << xvalue( argName ) << ' ' << err << '.';
    return user;
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::Context::Context( Client* client, bool silent )
    : _client ( client )
    , _user   ( client ? *connectedUsers[client->slot] : User::CONSOLE )
    , _silent ( silent )
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::Context::~Context()
{
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
