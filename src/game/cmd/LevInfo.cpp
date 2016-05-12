#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LevInfo::LevInfo()
    : AbstractBuiltin( "levinfo" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" );
    __descr << "Show admin level details.";
}

///////////////////////////////////////////////////////////////////////////////

LevInfo::~LevInfo()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LevInfo::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    Level& lev = lookupLEVEL( txt._args[1], txt );
    if (lev == Level::BAD)
        return PA_ERROR;

    InlineText cA;
    InlineText cB = xvalue;

    cA.flags |= ios::left;
    cA.width = 12;
    cA.suffix = ':';

    cB.prefixOutside = ' ';

    Buffer buf;
    buf << xheader( "-LEVEL INFORMATION" )
        << '\n' << cA( "level" )       << cB( lev.level )
        << '\n' << cA( "name" )        << cB( lev.namex )
        << '\n' << cA( "greet-text" )  << cB( lev.greetingText.empty() ? "none" : lev.greetingText )
        << '\n' << cA( "greet-audio" ) << cB( lev.greetingAudio.empty() ? "none" : lev.greetingAudio );

    doPrivileges( buf, lev.privGranted, lev.privDenied, cA, cB, "ACL entries" );

    print( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
LevInfo::doPrivileges(
    Buffer& buf,
    const PrivilegeSet& granted,
    const PrivilegeSet& denied,
    InlineText& cA,
    InlineText& cB,
    const string& name )
{
    buf << '\n' << cA( name );

    if (granted._indexName.empty() && denied._indexName.empty()) {
        buf << cB( "none" );
    }
    else {
        static const int nmax = 8; // max grants/denies per line
        int num = 3; // offset first line to make it shorter, and to avoid newline
        buf << xlindent;

        // process granted
        {
            const PrivilegeSet::IndexName::const_iterator max = granted._indexName.end();
            for ( PrivilegeSet::IndexName::const_iterator it = granted._indexName.begin(); it != max; it++ ) {
                if (num % nmax == 0)
                    buf << '\n';
                buf << cB( "+" + it->second->_name );
                num++;
            }
        }

        // process denied
        {
            const PrivilegeSet::IndexName::const_iterator max = denied._indexName.end();
            for ( PrivilegeSet::IndexName::const_iterator it = denied._indexName.begin(); it != max; it++ ) {
                if (num % nmax == 0)
                    buf << '\n';
                buf << cB( "-" + it->second->_name );
                num++;
            }
        }

        buf << xlunindent;
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
