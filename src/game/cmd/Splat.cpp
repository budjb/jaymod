#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Splat::Splat()
    : AbstractBuiltin( "splat" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Make a specified player go Splat!";
}

///////////////////////////////////////////////////////////////////////////////

Splat::~Splat()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Splat::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    if (isNotOnTeamError( *target, txt ))
        return PA_ERROR;

    G_Damage( &target->gentity, NULL, NULL, NULL, NULL, 10000, DAMAGE_JAY_NO_PROTECTION, MOD_UNKNOWN );

    const User& targetUser = *connectedUsers[target->slot];
    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " went splat!";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
