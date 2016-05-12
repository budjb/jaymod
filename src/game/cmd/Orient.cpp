#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Orient::Orient()
    : AbstractBuiltin( "orient" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Orients a player's view.";
}

///////////////////////////////////////////////////////////////////////////////

Orient::~Orient()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Orient::doExecute( Context& txt )
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

    // bail if not disoriented
    const User& targetUser = *connectedUsers[target->slot];
    if (!target->gclient.sess.disoriented[1]) {
        txt._ebuf << xvalue( targetUser.namex ) << " is not disoriented.";
        return PA_ERROR;
    }

    // Orient them (flag them)
    target->gclient.sess.disoriented[1] = qfalse;

    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " is re-oriented.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
