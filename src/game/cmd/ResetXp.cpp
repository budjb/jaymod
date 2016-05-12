#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

ResetXp::ResetXp()
    : AbstractBuiltin( "resetxp" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Completely reset a specific player's XP.";
}

///////////////////////////////////////////////////////////////////////////////

ResetXp::~ResetXp()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
ResetXp::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    target->xpReset();

    const User& targetUser = *connectedUsers[target->slot];
    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << "'s XP has been reset.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
