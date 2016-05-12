#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Splata::Splata()
    : AbstractBuiltin( "splata" )
{
    __usage << xvalue( "!" + _name );
    __descr << "M-m-m-monster splat...!";
}

///////////////////////////////////////////////////////////////////////////////

Splata::~Splata()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Splata::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    int num = 0;
    for (int i = 0; i < level.numConnectedClients; i++) {
        gentity_t* p = g_entities + level.sortedClients[i];

        if (!ISONTEAM( p ))
            continue;

        G_Damage( p, NULL, NULL, NULL, NULL, 10000, DAMAGE_JAY_NO_PROTECTION, MOD_UNKNOWN );
        num++;
    }

    Buffer buf;
    buf << _name << ": " << xvalue( num ) << " player" << (num == 1 ? "" : "s") << " M-M-M-Monster splattered!";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
