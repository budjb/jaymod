#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Mute::Mute()
    : AbstractBuiltin( "mute" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "REASON" );
    __descr << "Mute someone so he/she cannot talk. No voice commands, nor chat, nor admin commands.";
}

///////////////////////////////////////////////////////////////////////////////

Mute::~Mute()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Mute::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    User& user = *connectedUsers[target->slot];
    if (user == txt._user) {
        txt._ebuf << "You cannot mute yourself.";
        return PA_ERROR;
    }

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    // bail if already muted
    if (user.muted) {
        txt._ebuf << "Player is already muted.";
        return PA_ERROR;
    }

    // compute reason text
    string reason;
    if (txt._args.size() > 2)
        str::concatArgs( txt._args, reason, 2 );

    if (reason.empty())
        reason = "none";

    G_MutePlayer( &target->gentity, txt._user.namex.c_str(), reason );
    trap_SendServerCommand( target->slot, "cp \"^xYou've been muted.\n\"" );

    Buffer buf;
    buf << _name << ": " << xvalue( user.namex ) << " was muted.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
