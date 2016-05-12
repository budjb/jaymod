#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Seen::Seen()
    : AbstractBuiltin( "seen" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "NAME" );
    __descr << "Find the last time a specific admin was seen on the server.";
}

///////////////////////////////////////////////////////////////////////////////

Seen::~Seen()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Seen::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    const time_t now = time( NULL );

    // create a temporary set for online checks
    typedef set<const User*> Online;
    Online online;

    for (int i = 0; i < level.numConnectedClients; i++)
        online.insert( connectedUsers[level.sortedClients[i]] );

    // now look for name matches
    const string name = SanitizeString( txt._args[1], false );

    if (name.empty()) {
        txt._ebuf << xvalue( "NAME" ) << " is empty.";
        return PA_ERROR;
    }

    string err;
    list<User*> users;
    if (userDB.fetchByName( name, users, err )) {
        txt._ebuf << xvalue( "NAME" ) << ' ' << err << ": " << xvalue( name ) << " .";
        return PA_ERROR;
    }

    Buffer buf;
    static const int maxOutput = 4;
    int outputCount = 0;

    const list<User*>::const_iterator max = users.end();
    for ( list<User*>::const_iterator it = users.begin(); it != max; it++ ) {
        if (++outputCount > maxOutput)
            break;

        if (outputCount > 1)
            buf << '\n';
        buf << _name << ": ";

        User& user = **it;
        if (online.find( &user ) != online.end()) {
            buf << xvalue( user.namex ) << " is currently online.";
            continue;
        }

        const time_t delta = now - user.timestamp;
        const string stime = str::toStringSecondsRemaining( delta, true );
        buf << xvalue( user.namex ) << ' ' << xvalue( stime ) << " ago.";
    }

    if (outputCount == 0)
        buf << _name << ": No match found.";
    else if (outputCount > maxOutput)
        buf << '\n' << xcheader << "--too many results. Please be more specific in your search.";

    printChat( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
