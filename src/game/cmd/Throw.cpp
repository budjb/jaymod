#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Throw::Throw()
    : AbstractBuiltin( "throw" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << _ovalue( "DISTANCE" );
    __descr << "Throw a specific player straight ahead.";
}

///////////////////////////////////////////////////////////////////////////////

Throw::~Throw()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Throw::doExecute( Context& txt )
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

    int dist = 1500;
    if (txt._args.size() > 2) {
        dist = atoi( txt._args[2].c_str() );
        if (dist < 1)
            dist = 1;
        else if (dist > 10000)
            dist = 10000;
    }

    gentity_t* const targetEnt = &target->gentity;

    float angle = DEG2RAD( targetEnt->client->ps.viewangles[YAW] );
    vec3_t flatforward;
    flatforward[0] = cos( angle );
    flatforward[1] = sin( angle );
    flatforward[2] = 0;

    vec3_t ofs;
    VectorScale( flatforward, dist, ofs );

    ofs[2] = 150;
    VectorAdd( targetEnt->client->ps.velocity, ofs, targetEnt->client->ps.velocity );

    const User& targetUser = *connectedUsers[target->slot];
    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << " was thrown.";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
