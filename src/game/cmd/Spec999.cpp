#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Spec999::Spec999()
    : AbstractBuiltin( "spec999" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Put everyone with 999 ping to Spectator team.";
}

///////////////////////////////////////////////////////////////////////////////

Spec999::~Spec999()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Spec999::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    int num = 0;
    for (int i = 0; i < level.numConnectedClients; i++) {
        gentity_t* p = g_entities + level.sortedClients[i];
        if (p->r.svFlags & SVF_BOT)
            continue;

        if (!ISONTEAM( p ))
            continue;

        if (p->client->ps.ping < 999)
            continue;

        SetTeam( p, "s", qtrue, (weapon_t)-1, (weapon_t)-1, qfalse );
        num++;
    }

    Buffer buf;
    buf << _name << ": " << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " moved to spectators.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
