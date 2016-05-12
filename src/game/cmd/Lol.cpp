#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Lol::Lol()
    : AbstractBuiltin( "lol" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << _ovalue( "PLAYER_FILTER" )
            << ' ' << _ovalue( "NUM_GRENADES" );

    __descr << "Rain grenades from the sky.";
}

///////////////////////////////////////////////////////////////////////////////

Lol::~Lol()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Lol::doExecute( Context& txt )
{
    static const int maxGrenades = 16;

    if (txt._args.size() > 3)
        return PA_USAGE;

    vector<Client*> targets;
    int numGrenades = 1;

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
        numGrenades = atoi( txt._args[2].c_str() );
        if (numGrenades < 1)
            numGrenades = 1;
        else if (numGrenades > maxGrenades)
            numGrenades = maxGrenades;
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

        run( c, numGrenades );
        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was lol'd.";
    }
    else {
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " lol'd.";
    }

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
Lol::run(Client& target, int numNades)
{
    vec3_t dir;
    int it, grenadeWPID, degPerNade;
    float x, y;
    gentity_t *nade;
    gentity_t* ent = &target.gentity;

    if (ent->client->sess.sessionTeam == TEAM_AXIS) {
            grenadeWPID = WP_GRENADE_LAUNCHER;
    } else {
            grenadeWPID = WP_GRENADE_PINEAPPLE;
    }

    if (numNades < 1) {
            numNades = 1;
    } else if (numNades > 32) {
            numNades = 32;
    }

    degPerNade = 360 / numNades;

    for (it = 0; it < numNades; it++) {
        // there's probly a better way to do this, but this works
        x = 200 * cos( DEG2RAD(degPerNade * it) );  
        y = 200 * sin( DEG2RAD(degPerNade * it) );
        VectorSet( dir, x, y, 30 );
        VectorSubtract( ent->client->ps.velocity, dir, dir );
        dir[2] = 60;
        nade = fire_grenade( ent, ent->r.currentOrigin, dir, grenadeWPID );
        nade->nextthink = level.time + 1000 + (rand() % 1000);
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
