#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Throwa::Throwa()
    : AbstractBuiltin( "throwa" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "DISTANCE" );
    __descr << "Throw all players straight ahead.";
}

///////////////////////////////////////////////////////////////////////////////

Throwa::~Throwa()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Throwa::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    int dist = 1500;
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

        float angle = DEG2RAD( p->client->ps.viewangles[YAW] );

        vec3_t flatforward;
        flatforward[0] = cos(angle);
        flatforward[1] = sin(angle);
        flatforward[2] = 0;

        vec3_t ofs;
        VectorScale( flatforward, dist, ofs );

        ofs[2] = 150;
        VectorAdd( p->client->ps.velocity, ofs, p->client->ps.velocity );

        num++;
    }

    Buffer buf;
    buf << _name << ": " << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " thrown.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
