#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

bool
__matchClients( const string& spec, vector<Client*>& clients )
{
    // Clean out the out vector
    clients.clear();

    // Clean the search
    string needle = SanitizeString( spec, true );
    if (needle.empty())
        return true;

    // If a number is provided, it might be a slot #
    bool slot = true;
    for (int i = 0, end = needle.length(); i < end; i++) {
        if (needle[i] < '0' || needle[i] > '9') {
            slot = false;
            break;
        }
    }

    // Attempt to use the slot number
    if (slot) {
        int slotNumber = atoi( spec.c_str() );
        if (slotNumber >= 0 && slotNumber < level.maxclients) {
            Client& client = g_clientObjects[slotNumber];
            if (client.gclient.pers.connected == CON_CONNECTED) {
                clients.push_back( &client );
                return false;
            }
        }
    }

    // Search all connected clients
    for (int i = 0; i < level.numConnectedClients; i++) {
        Client& client = g_clientObjects[level.sortedClients[i]];

        // Skip if not fully connected
        if (client.gclient.pers.connected != CON_CONNECTED)
            continue;

        // Sanitize the client's name
        string haystack = SanitizeString( string(client.gclient.pers.netname), true );

        // Search
        if (haystack.find( needle ) != string::npos)
            clients.push_back( &client );
    }

    return clients.empty();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

AbstractCommand*
commandForName( const string& name )
{
    string key = name;
    str::toLower( key );
    Registry::iterator found = registry.find( key );
    if (found != registry.end())
        return found->second;

    // exact match was not found so we search for unique partially-qualified
    int num = 0;
    const Registry::iterator max = registry.end();
    for ( Registry::iterator it = registry.begin(); it != max; it++ ) {
        if (it->first.find( key ) == 0) {
            found = it;
            num++;
        }
    }

    return (num == 1) ? found->second : NULL; 
}

///////////////////////////////////////////////////////////////////////////////

bool
entityHasPermission( const gentity_t* ent, const Privilege& priv )
{
    return (ent ? connectedUsers[ ent-g_entities ]->hasPrivilege( priv ) : true);
}

///////////////////////////////////////////////////////////////////////////////

int
levelForEntity( const gentity_t* ent )
{
    return (ent->client ? connectedUsers[ ent-g_entities ]->authLevel : 0);
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
lockTeams( Client* actor, const vector<string>& args, Buffer& ebuf, bool lock )
{
    if (args.size() != 2)
        return AbstractCommand::PA_USAGE;

    const qboolean toLock = lock ? qtrue : qfalse;
    const string cmd = lock ? "lock" : "unlock";

    string larg = args[1];
    str::toLower( larg );

    Buffer buf;
    buf << cmd << ": " << xcvalue;

    if (larg == "all") {
        teamInfo[TEAM_AXIS].team_lock = (TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse;
        teamInfo[TEAM_ALLIES].team_lock = (TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse;
        G_updateSpecLock(TEAM_AXIS, (TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse);
        G_updateSpecLock(TEAM_ALLIES, (TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse);

        if (lock)
            level.server_settings |= CV_SVS_LOCKSPECS;
        else
            level.server_settings &= ~CV_SVS_LOCKSPECS;

        buf << "All teams " << cmd << "ed.";
    }
    else if (larg == "r") {
        teamInfo[TEAM_AXIS].team_lock = (TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse;
        buf << "Axis team " << cmd << "ed.";
    }
    else if (larg == "b") {
        teamInfo[TEAM_ALLIES].team_lock = (TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse;
        buf << "Allied team " << cmd << "ed.";
    }
    else if (larg == "s") {
        G_updateSpecLock(TEAM_AXIS, (TeamCount(-1, TEAM_AXIS)) ? toLock : qfalse);
        G_updateSpecLock(TEAM_ALLIES, (TeamCount(-1, TEAM_ALLIES)) ? toLock : qfalse);

        if (lock)
            level.server_settings |= CV_SVS_LOCKSPECS;
        else
            level.server_settings &= ~CV_SVS_LOCKSPECS;

        buf << "Spectators " << cmd << "ed.";
    }
    else {
        return AbstractCommand::PA_USAGE;
    }

    if (lock)
        level.server_settings |= CV_SVS_LOCKTEAMS;
    else
        level.server_settings &= ~CV_SVS_LOCKTEAMS;

    printCpm( actor, buf, true );

    trap_SetConfigstring( CS_SERVERTOGGLES, va( "%d", level.server_settings ));
    return AbstractCommand::PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

bool
matchClient( const string& spec, Client*& client, Buffer& ebuf )
{
    vector<Client*> clients;
    if (__matchClients( spec, clients )) {
        ebuf << "No matching client found.";
        return true;
    }

    if (clients.size() > 1) {
        ebuf << "Ambiguous client name: " << xvalue( clients.size() ) << " clients found.";
        return true;
    }

    client = clients[0];
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
matchClients( const string& spec, vector<Client*>& clients, Buffer& ebuf )
{
    if (__matchClients( spec, clients )) {
        ebuf << "No matching clients found.";
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

/* Returns true if command was a valid command.
 */
bool
process( Client* actor, bool silent, string* str )
{
    if( !cvars::g_admin.ivalue )
        return false;

    // prepare Context
    AbstractCommand::Context txt( actor );

    // populate args
    if (str)
        Engine::args( txt._args, *str );
    else
        Engine::args( txt._args );

    if (txt._args.empty())
        return false;

    // check for various chat encapsulations and shift array if needed
    {
        const string& s = txt._args[0];
        if (s.empty())
            return false;

        // check for various chat encapsulations and shift array if needed
        if (s == "say") {
            txt._args.erase( txt._args.begin() );
            if (txt._args.empty())
                return false;
        }
        else if (s.find("say_") == 0 && txt._user.hasPrivilege( priv::base::commandChat )) {
            txt._args.erase( txt._args.begin() );
            if (txt._args.empty())
                return false;
        }
    }

    // finally check for !command prefix and strip if needed
    string& s = txt._args[0];
    if (s.empty() || s[0] != '!')
        return false;

    // strip leading char
    s.erase( 0, 1 );
    if (s.empty())
        return false;

    AbstractCommand* command = commandForName( s );
    if (!command) {
        Buffer buf;
        buf << xfail( "Uknown command: " ) << '!' << xvalue( s );
        printChat( NULL, buf );
        return false;
    }

    command->execute( txt );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
print( Client* client, const Buffer& buffer, bool broadcast )
{
    if (client) {
        trap_SendServerCommand( "xpr", buffer, broadcast ? -1 : client->slot );
        return;
    }

    if (broadcast)
        trap_SendServerCommand( "xpr", buffer, -1 );

    TerminalDevice dev;
    list<string> output;
    dev.format( buffer, output );
    const list<string>::iterator end = output.end();
    for ( list<string>::iterator it = output.begin(); it != end; it++ )
        trap_Print( it->c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
printChat( Client* client, const Buffer& buffer, bool broadcast )
{
    if (client) {
        trap_SendServerCommand( "xcr", buffer, broadcast ? -1 : client->slot );
        return;
    }

    if (broadcast)
        trap_SendServerCommand( "xcr", buffer, -1 );

    TerminalDevice dev;
    list<string> output;
    dev.format( buffer, output );
    const list<string>::iterator end = output.end();
    for ( list<string>::iterator it = output.begin(); it != end; it++ )
        trap_Print( it->c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
printCpm( Client* client, const Buffer& buffer, bool broadcast )
{
    if (client) {
        trap_SendServerCommand( "xmr", buffer, broadcast ? -1 : client->slot );
        return;
    }

    if (broadcast)
        trap_SendServerCommand( "xmr", buffer, -1 );

    TerminalDevice dev;
    list<string> output;
    dev.format( buffer, output );
    const list<string>::iterator end = output.end();
    for ( list<string>::iterator it = output.begin(); it != end; it++ )
        trap_Print( it->c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
printPm( Client* client, const Buffer& buffer, bool sound )
{
    if (!client)
        return;

    if (sound)
        trap_SendServerCommand( "pmr", buffer, client->slot );
    else
        trap_SendServerCommand( "psr", buffer, client->slot );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
