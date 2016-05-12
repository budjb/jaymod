#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Disorient::Disorient()
    : AbstractBuiltin( "disorient" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Disorients (flips) a player's view.";
}

///////////////////////////////////////////////////////////////////////////////

Disorient::~Disorient()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Disorient::doExecute( Context& txt )
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

    // bail if already disoriented
    const User& targetUser = *connectedUsers[target->slot];
    if (target->gclient.sess.disoriented[1]) {
        txt._ebuf << xvalue( targetUser.namex ) << " is already disoriented.";
        return PA_ERROR;
    }

    // Disorient them (flag them)
    target->gclient.sess.disoriented[1] = qtrue;

    // Tell the world
    Buffer buf;
    buf << "disorient: "
        << xcvalue << xnone( targetUser.namex ) << " is disoriented.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
