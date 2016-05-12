#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Launch::Launch()
    : AbstractBuiltin( "launch" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "DISTANCE" );
    __descr << "Launch a player straight up.";
}

///////////////////////////////////////////////////////////////////////////////

Launch::~Launch()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Launch::doExecute( Context& txt )
{
    if (txt._args.size() < 2 || txt._args.size() > 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    if (isNotOnTeamError( *target, txt ))
        return PA_ERROR;

    if (isPlayingDeadError( *target, txt ))
        return PA_ERROR;

    int dist = 2500;
    if (txt._args.size() > 2) {
        dist = atoi( txt._args[2].c_str() );
        if (dist < 1)
            dist = 1;
        else if (dist > 10000)
            dist = 10000;
    }

    gentity_t* const targetEnt = &target->gentity;

    targetEnt->client->ps.velocity[0] = 0;
    targetEnt->client->ps.velocity[1] = 0;
    targetEnt->client->ps.velocity[2] = dist;

    const User& targetUser = *connectedUsers[target->slot];
    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " was launched.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
