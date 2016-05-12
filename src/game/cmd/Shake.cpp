#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Shake::Shake()
    : AbstractBuiltin( "shake" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << _ovalue( "PLAYER_FILTER" )
            << ' ' << _ovalue( "SECONDS" );

    __descr << "Make players screen shake for a chosen period of time. Default is 1 second.";
}

///////////////////////////////////////////////////////////////////////////////

Shake::~Shake()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Shake::doExecute( Context& txt )
{
    static const int maxSeconds = 60;

    if (txt._args.size() > 3)
        return PA_USAGE;

    vector<Client*> targets;
    int numSeconds = 1;

    if (txt._args.size() > 1) {
        // bail if no valid targets
        if (lookupPLAYER( txt._args[1], txt, targets, "PLAYER_FILTER" ))
            return PA_ERROR;
    }
    else {
        // populate with all
        for (int i = 0; i < level.numConnectedClients; i++)
            targets.push_back( &g_clientObjects[i] );
    }

    // bail if no targets
    if (targets.empty()) {
        txt._ebuf << "No players found.";
        return PA_ERROR;
    }

    if (txt._args.size() > 2) {
        numSeconds = atoi( txt._args[2].c_str() );
        if (numSeconds < 1)
            numSeconds = 1;
        else if (numSeconds > maxSeconds)
            numSeconds = maxSeconds;
    }

    Client* const single = (targets.size() == 1) ? targets.front() : NULL;
    int num = 0;

    const vector<Client*>::iterator max = targets.end();
    for ( vector<Client*>::iterator it = targets.begin(); it != max; it++ ) {
        Client& c = **it;
        gentity_t* const ent = &c.gentity;

        if (single) {
            if (isHigherLevelError( c, txt ))
                return PA_ERROR;

            if (isNotOnTeamError( c, txt ))
                return PA_ERROR;
        }

        if (isHigherLevel( c, txt._client ))
            continue;

        if (!ISONTEAM( ent ))
            continue;

        ent->client->pers.shakeTime = level.time + (numSeconds * 1000);
        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was shaken.";
    }
    else {
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " shaken.";
    }

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
