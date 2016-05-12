#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unmute::Unmute()
    : AbstractBuiltin( "unmute" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Allow a player to talk again.";
}

///////////////////////////////////////////////////////////////////////////////

Unmute::~Unmute()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unmute::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail if no valid target
    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    // bail if not muted
    const User& targetUser = *connectedUsers[target->slot];
    if (!targetUser.muted) {
        txt._ebuf << "Player is already unmuted.";
        return PA_ERROR;
    }

    G_UnmutePlayer( &target->gentity );
    trap_SendServerCommand( target->slot, "cp \"^xYou've been unmuted.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " was unmuted.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
