#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Pip::Pip()
    : AbstractBuiltin( "pip" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "PLAYER_FILTER" );
    __descr << "Make players glitter like a little elf.";
}

///////////////////////////////////////////////////////////////////////////////

Pip::~Pip()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Pip::doExecute( Context& txt )
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

        run( c );
        num++;
    }

    Buffer buf;
    buf << _name << ": ";
    if (single) {
       const User& u = *connectedUsers[single->slot];
       buf << xvalue( u.namex ) << " was pip'd.";
    }
    else {
       buf << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " pip'd.";
    }

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
Pip::run( Client& target )
{
    gentity_t* vic = &target.gentity;

    gentity_t *pip;

    pip = G_TempEntity( vic->r.currentOrigin, EV_SPARKS );
    VectorCopy( vic->r.currentOrigin, pip->s.origin );
    VectorCopy( vic->r.currentAngles, pip->s.angles );
    pip->s.origin[2] -= 6;
    pip->s.density = 5000;
    pip->s.frame = 6000;
    pip->s.angles2[0] = 18;
    pip->s.angles2[1] = 18;
    pip->s.angles2[2] = .5;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
