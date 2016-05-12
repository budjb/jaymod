#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserList::UserList()
    : AbstractBuiltin( "userlist" )
{
    __usage << xvalue( "!" + _name )
           << ' ' << _ovalue( "-ip IP" )
           << ' ' << _ovalue( "-level LEVEL" )
           << ' ' << _ovalue( "-name NAME" )
           << ' ' << _ovalue( "-since SECONDS" );

    __descr << "List users in database.";
}

///////////////////////////////////////////////////////////////////////////////

UserList::~UserList()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserList::doExecute( Context& txt )
{
    // bail if nothing to display
    if (userDB.mapNAME.empty()) {
        txt._ebuf << "The user database is empty.";
        return PA_ERROR;
    }

    struct Filter {
        string ip;
        int    level;
        string name;
        int    since;
    };
    Filter filter = { "", -1, "", 0 };

    // parse filter options
    {
        const vector<string>::size_type max = txt._args.size();
        for ( vector<string>::size_type i = 1; i < max; i++ ) {
            // pairs of args are expected
            if ((max - i) < 2)
                return PA_USAGE;

            string s = txt._args[i];
            str::toLower( s );

            if (s == "-ip" ) {
                filter.ip = txt._args[++i];
            }
            else if (s == "-level") {
                const string& s = txt._args[++i];
                string err;
                Level& lev = lookupLEVEL( s, txt );
                if (lev == Level::BAD)
                    return PA_ERROR;
                filter.level = lev.level;
            }
            else if (s == "-name") {
                filter.name = txt._args[++i];
                str::toLower( filter.name );
            }
            else if (s == "-since") {
                filter.since = str::toSeconds( txt._args[++i] );
            }
            else {
                return PA_USAGE;
            }
        }
    }

    InlineText cID    = xheader;
    InlineText cName  = xheader;
    InlineText cLevel = xheader;
    InlineText cIP    = xheader;
    InlineText cWhen  = xheader;

    cID.flags    |= ios::left;
    cName.flags  |= ios::left;
    cLevel.flags |= ios::left;
    cIP.flags    |= ios::left;
    cWhen.flags  |= ios::left;

    cID.width    = 8;
    cName.width  = 25;
    cLevel.width = 18;
    cIP.width    = 15;

    cName.prefixOutside  = ' ';
    cLevel.prefixOutside = ' ';
    cIP.prefixOutside    = ' ';
    cWhen.prefixOutside  = ' ';

    Buffer buf;
    buf << cID    ( "ID" )
        << cName  ( "NAME" )
        << cLevel ( "LEVEL" )
        << cIP    ( "IP" )
        << cWhen  ( "SEEN" );

    cID.color    = xcnone;
    cName.color  = xcnone;
    cLevel.color = xcnone;
    cIP.color    = xcnone;
    cWhen.color  = xcnone;

    const time_t now = time( NULL );
    string tmp;

    uint32 num = 0;
    const UserDB::mapNAME_t::const_iterator max = userDB.mapNAME.end();
    for ( UserDB::mapNAME_t::const_iterator it = userDB.mapNAME.begin(); it != max; it++ ) {
        const User& user = *it->second;
        const string id = (user.guid.length() == 32) ? user.guid.substr( 24 ) : "";

        if (!filter.ip.empty() && (user.ip.find( filter.ip ) == string::npos))
            continue;

        if (filter.level != -1 && user.authLevel != filter.level)
            continue;

        if (!filter.name.empty()) {
            tmp = user.name;
            str::toLower( tmp );
            if (tmp.find( filter.name ) == string::npos)
                continue;
        }

        if (filter.since > 0 && ((now - user.timestamp) > filter.since))
            continue;

        if (++num > (Page::maxLines * Page::maxPages))
            break;

        buf << '\n'
            << cID    ( id )
            << cName  ( str::etAlignLeft( user.namex, cName.width, tmp ));

        string err;
        Level& lev = levelDB.fetchByKey( user.authLevel, err );
        if (lev.namex.empty())
            buf << cLevel( lev.level );
        else
            buf << cLevel( str::etAlignLeft( lev.namex, cLevel.width, tmp ));

        char ftbuf[32];
		strftime( ftbuf, sizeof(ftbuf), "%a %b %d %H:%M:%S", localtime( &user.timestamp ));
        buf << cIP   ( user.ip )
            << cWhen ( ftbuf );
    }

    Page::report( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
