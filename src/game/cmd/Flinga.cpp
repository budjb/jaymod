#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Flinga::Flinga()
    : AbstractBuiltin( "flinga" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "DISTANCE" );
    __descr << "Fling all players randomly into the air.";
}

///////////////////////////////////////////////////////////////////////////////

Flinga::~Flinga()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Flinga::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    int dist = 2500;
    if (txt._args.size() > 1) {
        dist = atoi( txt._args[1].c_str() );
        if (dist < 1)
            dist = 1;
        else if (dist > 10000)
            dist = 10000;
    }

    int num = 0;
    for (int i = 0; i < level.numConnectedClients; i++) {
        gentity_t* p = g_entities + level.sortedClients[i];

        if (!ISONTEAM( p ))
            continue;

        if (p->health <= 0)
            continue;

        if (p->client->ps.eFlags & EF_PLAYDEAD)
            continue;

        p->client->ps.velocity[0] = crandom() * dist;
        p->client->ps.velocity[1] = crandom() * dist;
        p->client->ps.velocity[2] = crandom() * dist;
        if (p->client->ps.velocity[2] < 0)
            p->client->ps.velocity[2] *= -1;

        num++;
    }

    Buffer buf;
    buf << _name << ": " << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " flung.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
