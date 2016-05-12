#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserInfo::UserInfo()
    : AbstractBuiltin( "userinfo" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Show user details.";
}

///////////////////////////////////////////////////////////////////////////////

UserInfo::~UserInfo()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserInfo::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail on invalid id
    User& user = lookupUSER( txt._args[1], txt );
    if (user == User::BAD)
        return PA_ERROR;

    InlineText cA;
    InlineText cB = xvalue;

    cA.flags |= ios::left;
    cA.width = 12;
    cA.suffix = ':';

    cB.prefixOutside = ' ';

    Buffer buf;
    doUser( buf, user, cA, cB );

    print( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
UserInfo::doUser( Buffer& buf, const User& user, InlineText& cA, InlineText& cB )
{
    string err;
    const Level& lev = levelDB.fetchByKey( user.authLevel, err );

    time_t now = time( NULL );
    char ftime[32];
    strftime( ftime, sizeof(ftime), "%c", localtime( &now ));

    buf << xheader( "-USER INFORMATION" )
        << '\n' << cA( "name" ) << cB( user.namex );

    if (lev.namex.empty())
        buf << '\n' << cA( "level" ) << cB( lev.level );
    else
        buf << '\n' << cA( "level" ) << cB( lev.level ) << " (" << xvalue( lev.namex ) << ')';

        buf << '\n' << cA( "GUID"        ) << cB( user.guid )
            << " (USERID: " << xvalue( user.guid.length() == 32 ? user.guid.substr( 24 ) : "" ) << ')'
        << '\n' << cA( "IP"          ) << cB( user.ip )
        << '\n' << cA( "MAC"         ) << cB( user.mac )
        << '\n' << cA( "timestamp"   ) << cB( ftime )
        << '\n' << cA( "greet-text"  ) << cB( user.greetingText.empty() ? "none" : user.greetingText )
        << '\n' << cA( "greet-audio" ) << cB( user.greetingAudio.empty() ? "none" : user.greetingAudio  );

    if (user.muted) {
        buf << '\n' << xheader( "-MUTE INFORMATION" );

        char ftbuf[32];
        strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &user.muteTime ));
        buf << '\n' << cA( "when" ) << cB( ftbuf );

        if (user.banExpiry) {
            strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &user.banExpiry ));
            buf << '\n' << cA( "expiry" ) << cB( ftbuf )
                << " (" << cB( str::toStringSecondsRemaining( user.banExpiry )) << ')';
        }
        else {
            buf << '\n' << cA( "expiry" ) << cB( "PERMANENT" );
        }

        buf << '\n' << cA( "reason" )    << cB( user.muteReason )
            << '\n' << cA( "authority" ) << cB( user.muteAuthorityx );
    }

    PrivilegeSet pseudo;
    const PrivilegeSet* granted = user.privGranted;
    const PrivilegeSet* denied  = user.privDenied;

    if (!granted)
        granted = &pseudo;
    if (!denied)
        denied = &pseudo;

    LevInfo::doPrivileges( buf, *granted, *denied, cA, cB, "private-ACL" );

    if (!user.notes.empty()) {
        const vector<string>::size_type max = user.notes.size();
        for ( vector<string>::size_type i = 0; i < max; i++ ) {
            ostringstream oss;
            oss << "note " << (i+1);
            buf << '\n' << cA( oss.str() ) << cB( user.notes[i] );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
