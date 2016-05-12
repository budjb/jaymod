#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Pop::Pop()
    : AbstractBuiltin( "pop" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "PLAYER_FILTER" );
    __descr << "Let all players pop off their helmets.";
}

///////////////////////////////////////////////////////////////////////////////

Pop::~Pop()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Pop::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    vector<Client*> targets;

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

           if (ent->client->ps.eFlags & EF_HEADSHOT) {
               const User& u = *connectedUsers[c.slot];
               txt._ebuf << xvalue( u.namex ) << " has no helmet.";
               return PA_ERROR;
           }
        }

        if (isHigherLevel( c, txt._client ))
            continue;

        if (!ISONTEAM( ent ))
            continue;

        if (ent->client->ps.eFlags & EF_HEADSHOT)
            continue;

        vec3_t fwd;
        AngleVectors( c.gclient.ps.viewangles, fwd, NULL, NULL );
        G_AddEvent( ent, EV_LOSE_HAT, DirToByte(fwd) );

        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was pop'd.";
    }
    else {
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " pop'd.";
    }

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
