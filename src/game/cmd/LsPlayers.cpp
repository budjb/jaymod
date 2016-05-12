#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LsPlayers::LsPlayers()
    : AbstractBuiltin( "lsplayers" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Display all players connected, their client numbers and admin levels.";
}

///////////////////////////////////////////////////////////////////////////////

LsPlayers::~LsPlayers()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LsPlayers::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    // bail if nothing to display
    if (level.numConnectedClients < 1) {
        txt._ebuf << "There are no players online.";
        return PA_ERROR;
    }

    InlineText colA = xheader;
    InlineText colB = xheader;
    InlineText colC = xheader;
    InlineText colD = xheader;
    InlineText colE = xheader;
    InlineText colF = xheader;
    InlineText colG = xheader;
    InlineText colH = xheader;
    InlineText colI = xheader;

    colC.flags |= ios::left;
    colD.flags |= ios::left;
    colE.flags |= ios::left;
    colF.flags |= ios::left;

    colA.width = 2;
    colB.width = 1;
    colC.width = 1;
    colD.width = 31;
    colE.width = 15;
    colF.width = 6;
    colG.width = 6;
    colH.width = 4;
    colI.width = 2;

    colB.prefixOutside = " ";
    colD.prefixOutside = " ";
    colE.prefixOutside = " ";
    colF.prefixOutside = " ";
    colG.prefixOutside = " ";
    colH.prefixOutside = " ";
    colI.prefixOutside = " ";

    Buffer buf;
    buf << colA( "##" )
        << colB( "C" )
        << colC( "M" )
        << colD( "NAME" )
        << colE( "LEVEL" )
        << colF( "TEAM" )
        << colG( "XP" )
        << colH( "PING" )
        << colI( "NC" )
        << '\n';

    colA.color = xcnone;
    colB.color = xcnone;
    colC.color = xcnone;
    colD.color = xcnone;
    colE.color = xcnone;
    colF.color = xcnone;
    colG.color = xcnone;
    colH.color = xcnone;
    colI.color = xcnone;

    string tmp;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        User& user = *connectedUsers[i];
        if (user == User::BAD)
            continue;

        Client& cobj = g_clientObjects[i];

        string err;
        Level& lev = levelDB.fetchByKey( user.authLevel, err );
        gclient_t& client = level.clients[i];

        buf << colA( i )
            << colB( (client.pers.connected == CON_CONNECTING ? 'C' : '-') )
            << colC( (user.muted ? 'M' : '-') )
            << colD( str::etAlignLeft( user.namex, colD.width, tmp ));

        if (lev.namex.empty())
            buf << colE( lev.level );
        else
            buf << colE( str::etAlignLeft( lev.namex, colE.width, tmp ));

        switch (client.sess.sessionTeam) {
            case TEAM_ALLIES:
                colF.color = xcblue;
                buf << colF( "allies" );
                break;

            case TEAM_AXIS:
                colF.color = xcred;
                buf << colF( "axis" );
                break;

            case TEAM_SPECTATOR:
            default:
                colF.color = xcyellow;
                buf << colF( "spec" );
                break;
        }

        // FIELD: XP
        float xp = 0.0f;
        for (int si = 0; si < SK_NUM_SKILLS; si++)
            xp += client.sess.skillpoints[si];

        buf << colG( int(xp) )
            << colH( client.ps.ping )
            << colI( cobj.numNameChanges > 99 ? 99 : cobj.numNameChanges )
            << '\n';
    }

    print( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
