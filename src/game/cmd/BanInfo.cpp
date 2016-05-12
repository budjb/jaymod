#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

BanInfo::BanInfo()
    : AbstractBuiltin( "baninfo" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Show ban details.";
}

///////////////////////////////////////////////////////////////////////////////

BanInfo::~BanInfo()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
BanInfo::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail on invalid id
    const string id = txt._args[1];
    User& user = lookupUSER( id, txt );
    if (user == User::BAD)
        return PA_ERROR;

    string err;
    const Level& lev = levelDB.fetchByKey( user.authLevel, err );

    InlineText colA;
    InlineText colB = xvalue;

    colA.flags  |= ios::left;
    colA.width   = 11;
    colA.suffix  = ":";

    Buffer buf;
    buf << xheader( "-BAN INFORMATION" );

    buf << '\n' << colA( "ID" )   << colB( id )
        << '\n' << colA( "name" ) << colB( user.namex );

    if (lev.namex.empty())
        buf << '\n' << colA( "level" ) << colB( lev.level );
    else
        buf << '\n' << colA( "level" ) << colB( lev.namex ) << " (" << xvalue( lev.level ) << ')';

    buf << '\n' << colA( "GUID" ) << colB( user.guid )
        << '\n' << colA( "IP"   ) << colB( user.ip )
        << '\n' << colA( "MAC"  ) << colB( user.mac );

    char ftbuf[32];
    strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &user.banTime ));
    buf << '\n' << colA( "when" ) << colB( ftbuf );

    if (user.banExpiry) {
        strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &user.banExpiry ));
        buf << '\n' << colA( "expiry" ) << colB( ftbuf )
            << " (" << colB( str::toStringSecondsRemaining( user.banExpiry )) << ')';
    }
    else {
        buf << '\n' << colA( "expiry" ) << colB( "PERMANENT" );
    }

    buf << '\n' << colA( "authority" ) << colB( user.banAuthorityx )
        << '\n' << colA( "reason" )    << colB( user.banReason );

    print( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
