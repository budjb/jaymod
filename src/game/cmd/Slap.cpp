#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Slap::Slap()
    : AbstractBuiltin( "slap" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << _ovalue( "PLAYER_FILTER" )
            << ' ' << _ovalue( "DAMAGE" );

    __descr << "Slap players around.";
}

///////////////////////////////////////////////////////////////////////////////

Slap::~Slap()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Slap::doExecute( Context& txt )
{
    static const int maxDamage = 250;

    if (txt._args.size() > 3)
        return PA_USAGE;

    vector<Client*> targets;
    int numDamage = 1;

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
        numDamage = atoi( txt._args[2].c_str() );
        if (numDamage < 1)
            numDamage = 1;
        else if (numDamage > maxDamage)
            numDamage = maxDamage;
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

            if (isDeadError( c, txt ))
                return PA_ERROR;
        }

        if (isHigherLevel( c, txt._client ))
            continue;

        if (!ISONTEAM( ent ))
            continue;

        if (ent->health <= 0)
            continue;

        // slap the biatch
        G_ClientSound( ent,"sound/jaymod/slap.wav" );
        G_ShakeClient( ent, 1000000 );
        ent->health -= numDamage;
        if (ent->health < 1)
            ent->health = 1;

        // also play sound for txt._client
        if (txt._client)
            G_ClientSound( g_entities + txt._client->slot, "sound/jaymod/slap.wav" );

        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was slapped.";
    }
    else
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " slapped.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
