#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

PutTeam::PutTeam()
    : AbstractBuiltin( "putteam" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" )
            << " <" << xvalue( "r" ) << '|' << xvalue( "b" ) << '|' << xvalue( "s" ) << '>';

    __descr << "Put a specific player on a different team.";
}

///////////////////////////////////////////////////////////////////////////////

PutTeam::~PutTeam()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
PutTeam::doExecute( Context& txt )
{
    if (txt._args.size() != 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    team_t team;
    string steam = txt._args[2];
    str::toLower( steam );

    if (steam == "r" || steam == "axis") {
        team = TEAM_AXIS;
        steam = "r";
    }
    else if (steam == "b" || steam == "allies") {
        team = TEAM_ALLIES;
        steam = "b";
    }
    else if (steam == "s" || steam.find( "spec" ) == 0) {
        team = TEAM_SPECTATOR;
        steam = "s";
    }
    else {
        return PA_USAGE;
    }
    
    const User& targetUser = *connectedUsers[target->slot];
    gentity_t* const targetEnt = &target->gentity;
    if (targetEnt->client->sess.sessionTeam == team) {
        txt._ebuf << xvalue( targetUser.namex ) << " is already on " << str::toString( team );
        return PA_ERROR;
    }

    if (!SetTeam( targetEnt, steam.c_str(), qtrue, (weapon_t)-1, (weapon_t)-1, qfalse )) {
        txt._ebuf << "Unable to move player to team.";
        return PA_ERROR;
    }

    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " moved to " << str::toString( team );

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
