#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Revive::Revive()
    : AbstractBuiltin( "revive" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "PLAYER" );
    __descr << "Revive yourself, or revive a different player.";
}

///////////////////////////////////////////////////////////////////////////////

Revive::~Revive()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Revive::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    Client* target;
    if (txt._args.size() == 1) {
        if (txt._user == User::CONSOLE) {
            txt._ebuf << xvalue( txt._user.namex ) << " cannot be revived.";
            return PA_ERROR;
        }
        target = txt._client;
    }
    else {
        if (lookupPLAYER( txt._args[1], txt, target ))
            return PA_ERROR;
    }

    if (isAliveError( *target, txt ))
        return PA_ERROR;

    ReviveEntity( txt._client ? &txt._client->gentity : &target->gentity, &target->gentity );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
