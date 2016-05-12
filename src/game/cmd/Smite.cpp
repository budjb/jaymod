#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Smite::Smite()
    : AbstractBuiltin( "smite" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "PLAYER_FILTER" );
    __descr << "Set players on fire.";
}

///////////////////////////////////////////////////////////////////////////////

Smite::~Smite()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Smite::doExecute( Context& txt )
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
        }

        if (isHigherLevel( c, txt._client ))
            continue;

        if (!ISONTEAM( ent ))
            continue;

        G_BurnMeGood( ent, ent, NULL );
        trap_SendServerCommand( ent-g_entities, "cp \"^1Burn baby burn!\"" );

        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was burned.";
    }
    else {
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " burned.";
    }

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
