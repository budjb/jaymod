#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

BanList::BanList()
    : AbstractBuiltin( "banlist" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << _ovalue( "-ip IP" )
            << ' ' << _ovalue( "-level LEVEL" )
            << ' ' << _ovalue( "-name NAME" )
            << ' ' << _ovalue( "-since SECONDS" )
            << ' ' << _ovalue( "-auth AUTHORITY" );

    __descr << "List banned users.";
}

///////////////////////////////////////////////////////////////////////////////

BanList::~BanList()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
BanList::doExecute( Context& txt )
{
    // bail if nothing to display
    if (userDB.mapBANTIME.empty()) {
        txt._ebuf << "There are no banned users.";
        return PA_ERROR;
    }

    struct Filter {
        string auth;  
        string ip;
        int    level;
        string name;  
        int    since; 
    };
    Filter filter = { "", "", -1, "", 0 };

    // parse filter options    
    {
        const vector<string>::size_type max = txt._args.size();
        for ( vector<string>::size_type i = 1; i < max; i++ ) {
            // pairs of args are expected
            if ((max - i) < 2)
                return PA_USAGE;

            string s = txt._args[i];
            str::toLower( s );

            if (s == "-auth" ) {
                filter.auth = txt._args[++i];
                str::toLower( filter.auth );
            }
            else if (s == "-ip" ) {
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

    InlineText cID      = xheader;
    InlineText cWhen    = xheader;
    InlineText cSubject = xheader;
    InlineText cRemain  = xheader;
    InlineText cAuth    = xheader;

    cID.flags      |= ios::left;
    cWhen.flags    |= ios::left;
    cSubject.flags |= ios::left;

    cID.width      = 8;
    cWhen.width    = 24;
    cSubject.width = 25;
    cRemain.width  = 13;

    cWhen.prefixOutside    = ' ';
    cSubject.prefixOutside = ' ';
    cRemain.prefixOutside  = ' ';
    cAuth.prefixOutside    = ' ';

    Buffer buf;
    buf << cID      ( "ID" )
        << cWhen    ( "WHEN" )
        << cSubject ( "SUBJECT" )
        << cRemain  ( "REMAINING" )
        << cAuth    ( "AUTHORITY" );

    cID.color      = xcnone;
    cWhen.color    = xcnone;
    cSubject.color = xcnone;
    cRemain.color  = xcnone;
    cAuth.color    = xcnone;

    const time_t now = time( NULL );
    string tmp;

    uint32 numExpired = 0;
    uint32 num = 0;
    const UserDB::mapBANTIME_t::const_reverse_iterator max = userDB.mapBANTIME.rend();
    for ( UserDB::mapBANTIME_t::const_reverse_iterator it = userDB.mapBANTIME.rbegin(); it != max; it++ ) {
        const User& user = *it->second;
        const string id = (user.guid.length() == 32) ? user.guid.substr( 24 ) : "";

        const time_t deltaTime = user.banExpiry - now;

        // skip if ban has expired
        if (user.banExpiry && deltaTime < 0) {
            numExpired++;
            continue;
        }

        if (!filter.auth.empty()) {
            tmp = user.banAuthority;
            str::toLower( tmp );
            if (tmp.find( filter.auth ) == string::npos)
                continue;
        }

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

        // format ban time
        char ftime[32];
        strftime( ftime, sizeof(ftime), "%c", localtime( &user.banTime ));

        buf << '\n'
            << cID      ( id )
            << cWhen    ( ftime )
            << cSubject ( str::etAlignLeft( user.namex, cSubject.width, tmp ));

        ostringstream remain;
        if (!user.banExpiry) {
            remain << "permanent";
        }
        else {
            int secs = (user.banExpiry - now);

            int days = secs / (60*60*24);
            secs -= (days * (60*60*24));

            int hours = secs / (60*60);
            secs -= (hours * (60*60));

            int mins = secs / 60;
            secs -= (mins * 60);

            if (days > 999)
                remain << "999d-";
            else if (days > 0)
                remain << days << "d-";

            remain << setw(2) << setfill('0') << hours
                   << ':' << setw(2) << setfill('0') << mins
                   << ':' << setw(2) << setfill('0') << secs;
        }

        buf << cRemain ( remain.str() )
            << cAuth   ( user.banAuthorityx );
    }

    Page::report( txt._client, buf );

    if (numExpired) {
        buf.reset();
        buf << xcheader << "--there were " << xvalue( numExpired ) << " expired-bans not listed.";
        print( txt._client, buf );
    }

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
