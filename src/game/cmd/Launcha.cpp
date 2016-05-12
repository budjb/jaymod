#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Launcha::Launcha()
    : AbstractBuiltin( "launcha" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "DISTANCE" );
    __descr << "Launch all players straight up.";
}

///////////////////////////////////////////////////////////////////////////////

Launcha::~Launcha()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Launcha::doExecute( Context& txt )
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

        p->client->ps.velocity[0] = 0;
        p->client->ps.velocity[1] = 0;
        p->client->ps.velocity[2] = dist;

        num++;
    }

    Buffer buf;
    buf << _name << ": "
        << xvalue( num )
        << " player" << (num == 1 ? "" : "s")
        << " launched.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
