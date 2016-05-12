#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Finger::Finger()
    : AbstractBuiltin( "finger" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" );
    __descr << "Get information about a player.";
}

///////////////////////////////////////////////////////////////////////////////

Finger::~Finger()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Finger::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    const User& user = *connectedUsers[target->slot];

    InlineText cA;
    InlineText cB = xvalue;

    cA.flags  |= ios::left;
    cA.width   = 13;
    cA.suffix  = ":";

    cB.prefixOutside = ' ';

    Buffer buf;
    buf << xheader( "-PLAYER INFORMATION" )
        << '\n' << cA( "slot" ) << cB( target->slot )
        << '\n' << cA( "team" ) << cB( str::toString( target->gclient.sess.sessionTeam ))
        << '\n' << cA( "name changes" ) << cB( target->numNameChanges );

    buf << '\n';
    UserInfo::doUser( buf, user, cA, cB );

    print( txt._client, buf );
        return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
