#include <bgame/impl.h>

namespace {
using namespace text;

///////////////////////////////////////////////////////////////////////////////

namespace {
    typedef map<int,User*> TeamMap;

    struct Team {
        InlineText colSlot;
        InlineText colLevel;
        InlineText colName;
        TeamMap    members;
    } teams[4];
}

///////////////////////////////////////////////////////////////////////////////

void outputRow( Buffer& buf, Team& left, Team& right )
{
    string tmp;
    for (;;) {
        bool output = false;

        TeamMap::iterator it = left.members.begin();
        if (it != left.members.end()) {
            output = true;
            buf << '\n'
                << left.colSlot  ( it->first )
                << left.colLevel ( it->second->authLevel )
                << left.colName  ( str::etAlignLeft( it->second->namex, left.colName.width, tmp ));

            left.members.erase( it );
        }

        it = right.members.begin();
        if (it != right.members.end()) {
            if (!output) {
                buf << '\n'
                    << left.colSlot  ( "" )
                    << left.colLevel ( "" )
                    << left.colName  ( "" );
            }
            output = true;
            buf << right.colSlot  ( it->first )
                << right.colLevel ( it->second->authLevel )
                << right.colName  ( str::etAlignLeft( it->second->namex, right.colName.width, tmp ));

            right.members.erase( it );
        }
        else if (output) {
            buf << right.colSlot  ( "" )
                << right.colLevel ( "" )
                << right.colName  ( "" );
        }

        if (!output)
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namepsace anonymous

///////////////////////////////////////////////////////////////////////////////

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

ListPlayers::ListPlayers()
    : AbstractBuiltin( "listplayers" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Display all players connected, their client numbers and admin levels.";
}

///////////////////////////////////////////////////////////////////////////////

ListPlayers::~ListPlayers()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
ListPlayers::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    if (level.numConnectedClients == 0) {
       Buffer buf;
       buf << _name << ": " << "No players connected.";
       printChat( txt._client, buf );
       return PA_NONE;
    }

    // initialize team data
    for (int i = 0; i < 4; i++) {
        Team& t = teams[i];

        t.colSlot  = xvalue;
        t.colLevel = xvalue;
        t.colName  = xvalue;

        t.colSlot.width = 1;
        t.colLevel.width = 1;

        t.colName.flags |= ios::left;
        t.colName.width = 15;
    }

    // convience refs
    Team& axis   = teams[0];
    Team& allies = teams[1];
    Team& specsL = teams[2];
    Team& specsR = teams[3];

    axis.colSlot.prefixOutside  = "| ";
    axis.colLevel.prefixOutside = " | ";
    axis.colName.prefixOutside  = " | ";

    allies.colSlot.prefixOutside  = " | ";
    allies.colLevel.prefixOutside = " | ";
    allies.colName.prefixOutside  = " | ";
    allies.colName.suffixOutside  = " |";

    specsL.colSlot = axis.colSlot;
    specsL.colLevel = axis.colLevel;
    specsL.colName = axis.colName;

    specsR.colSlot = allies.colSlot;
    specsR.colLevel = allies.colLevel;
    specsR.colName = allies.colName;

    // build data
    bool specLeft = true;
    for (int i = 0; i < level.numConnectedClients; i++ ) {
        const int slot = level.sortedClients[i];
        gentity_t& p = *(g_entities + slot);
        User& user = *connectedUsers[slot];

        Team* team;
        switch (p.client->sess.sessionTeam) {
            case TEAM_AXIS:
                team = &axis;
                break;

            case TEAM_ALLIES:
                team = &allies;
                break;

            default:
            case TEAM_SPECTATOR:
                team = specLeft ? &specsL : &specsR;
                specLeft = !specLeft;
                break;
        }

        team->members[slot] = connectedUsers[slot];

        static const float lf10 = logf(10);

        if (team->colSlot.width < ((logf(slot) / lf10) + 1))
            team->colSlot.width = int((logf(slot) / lf10) + 1);

        if (team->colLevel.width < ((logf(user.authLevel) / lf10) + 1))
            team->colLevel.width = int((logf(user.authLevel) / lf10) + 1);

        if (team->colName.width < int(str::etLength( user.namex )))
            team->colName.width = int(str::etLength( user.namex ));
    }

    if (axis.colSlot.width < specsL.colSlot.width)
        axis.colSlot.width = specsL.colSlot.width;
    else if (specsL.colSlot.width < axis.colSlot.width)
        specsL.colSlot.width = axis.colSlot.width;

    if (axis.colLevel.width < specsL.colLevel.width)
        axis.colLevel.width = specsL.colLevel.width;
    else if (specsL.colLevel.width < axis.colLevel.width)
        specsL.colLevel.width = axis.colLevel.width;

    if (axis.colName.width < specsL.colName.width)
        axis.colName.width = specsL.colName.width;
    else if (specsL.colName.width < axis.colName.width)
        specsL.colName.width = axis.colName.width;

    if (allies.colSlot.width < specsR.colSlot.width)
        allies.colSlot.width = specsR.colSlot.width;
    else if (specsR.colSlot.width < allies.colSlot.width)
        specsR.colSlot.width = allies.colSlot.width;

    if (allies.colLevel.width < specsR.colLevel.width)
        allies.colLevel.width = specsR.colLevel.width;
    else if (specsR.colLevel.width < allies.colLevel.width)
        specsR.colLevel.width = allies.colLevel.width;

    if (allies.colName.width < specsR.colName.width)
        allies.colName.width = specsR.colName.width;
    else if (specsR.colName.width < allies.colName.width)
        specsR.colName.width = allies.colName.width;

    string::size_type width =
          axis.colSlot.prefixOutside.length()  + axis.colSlot.width  + axis.colSlot.suffixOutside.length()
        + axis.colLevel.prefixOutside.length() + axis.colLevel.width + axis.colLevel.suffixOutside.length()
        + axis.colName.prefixOutside.length()  + axis.colName.width  + axis.colName.suffixOutside.length()

        + allies.colSlot.prefixOutside.length()  + allies.colSlot.width  + allies.colSlot.suffixOutside.length()
        + allies.colLevel.prefixOutside.length() + allies.colLevel.width + allies.colLevel.suffixOutside.length()
        + allies.colName.prefixOutside.length()  + allies.colName.width  + allies.colName.suffixOutside.length();

    string top;
    top += '/';
    top.append( width-2, '-' );
    top += '\\';

    string divider;
    divider += '|';
    divider.append( width-2, '-' );
    divider += '|';

    string bottom;
    bottom += '\\';
    bottom.append( width-2, '-' );
    bottom += '/';

    Buffer buf;
    buf << top;

    // output axis/allies
    bool needdiv = false;
    if (axis.members.size() || allies.members.size()) {
        needdiv = true;

        InlineText hlSlot  = axis.colSlot;
        InlineText hlLevel = axis.colLevel;
        InlineText hlName  = axis.colName;

        InlineText hrSlot  = allies.colSlot;
        InlineText hrLevel = allies.colLevel;
        InlineText hrName  = allies.colName;

        hlSlot.color  = xcheader;
        hlLevel.color = xcheader;
        hlName.color  = xcheader;

        hrSlot.color  = xcheader;
        hrLevel.color = xcheader;
        hrName.color  = xcheader;

        buf << '\n'
            << hlSlot( '#' )  << hlLevel( "L" ) << hlName( "AXIS" )
            << hrSlot( '#' )  << hrLevel( 'L' ) << hrName( "ALLLIES" )
            << '\n' << divider;
        outputRow( buf, axis, allies );
    }

    // output spectators left/right
    if (specsL.members.size() || specsR.members.size()) {
        const string title = "SPECTATORS";

        InlineText head = xheader;

        head.flags |= ios::left;
        head.prefixOutside = specsL.colSlot.prefixOutside;
        head.suffixOutside = specsR.colName.suffixOutside;
        head.width = width - specsL.colSlot.prefixOutside.length() - specsR.colName.suffixOutside.length();

        if (needdiv)
            buf << '\n' << divider;

        // center spec title
        string s;
        s.append( (head.width - title.length()) / 2, ' ' );
        s += title;

        buf << '\n' << head( s )
            << '\n' << divider;
        outputRow( buf, specsL, specsR );
    }

    buf << '\n' << bottom;
    print( txt._client, buf );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
