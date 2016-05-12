#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Pants::Pants()
    : AbstractBuiltin( "pants" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Strip a player of their pants.";
}

///////////////////////////////////////////////////////////////////////////////

Pants::~Pants()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Pants::doExecute( Context& txt )
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


    // bail if already stripped
    const User& targetUser = *connectedUsers[target->slot];
    gentity_t* const targetEnt = &target->gentity;
    if (G_HasJayFlag( targetEnt, 1, JF_LOSTPANTS )) {
        txt._ebuf << xvalue( targetUser.namex ) << " is already stripped.";
        return PA_ERROR;
    }

    G_SetJayFlag( targetEnt, 1, JF_LOSTPANTS );

    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " was stripped.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
