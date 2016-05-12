#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Spec::Spec()
    : AbstractBuiltin( "spec", true )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Instantly switch to Spectator team and follow the specified player.";
}

///////////////////////////////////////////////////////////////////////////////

Spec::~Spec()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Spec::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail if console
    if (txt._user == User::CONSOLE) {
        txt._ebuf << "You are on the console.";
        return PA_ERROR;
    }

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    if (isNotOnTeamError( *target, txt ))
        return PA_ERROR;

    // bail if self
    if (target == txt._client) {
        txt._ebuf << "You cannot spectate yourself.";
        return PA_ERROR;
    }

    // change team if not already on spec
    gentity_t& actorEnt = txt._client->gentity;
    if ( actorEnt.client->sess.sessionTeam != TEAM_SPECTATOR )
        SetTeam( &actorEnt, "spectator", qfalse, (weapon_t)-1, (weapon_t)-1, qfalse );

    gentity_t* const targetEnt = &target->gentity;

    // follow
    actorEnt.client->sess.spectatorState = SPECTATOR_FOLLOW;
    actorEnt.client->sess.spectatorClient = targetEnt->s.number;

    const User& targetUser = *connectedUsers[target->slot];
    Buffer buf;
    buf << _name << ": " << "Now following " << xvalue( targetUser.namex ) << " .";
    printChat( txt._client, buf );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
