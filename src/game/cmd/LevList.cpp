#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LevList::LevList()
    : AbstractBuiltin( "levlist" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "-name NAME" );
    __descr << "List admin levels in database.";
}

///////////////////////////////////////////////////////////////////////////////

LevList::~LevList()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LevList::doExecute( Context& txt )
{
    // bail if nothing to display
    if (levelDB.mapLEVEL.empty()) {
        txt._ebuf << "The admin level database is empty.";
        return PA_ERROR;
    }

    struct Filter {
        string name;
    };
    Filter filter = { "" };

    // parse filter options
    {
        const vector<string>::size_type max = txt._args.size();
        for ( vector<string>::size_type i = 1; i < max; i++ ) {
            // pairs of args are expected
            if ((max - i) < 2)
                return PA_USAGE;

            string s = txt._args[i];
            str::toLower( s );

            if (s == "-name") {
                filter.name = txt._args[++i];
                str::toLower( filter.name );
            }
            else {
                return PA_USAGE;
            }
        }
    }


    InlineText cNumber = xheader;
    InlineText cName   = xheader;
    InlineText cText   = xheader;
    InlineText cAudio  = xheader;
    InlineText cACL    = xheader;

    cName.flags  |= ios::left;
    cText.flags  |= ios::left;
    cAudio.flags |= ios::left;

    cNumber.width = 3;
    cName.width   = 15;
    cText.width   = 3;
    cAudio.width  = 3;

    cName.prefixOutside  = ' ';
    cText.prefixOutside  = ' ';
    cAudio.prefixOutside = ' ';
    cAudio.suffixOutside = ' ';  // extra space
    cACL.prefixOutside   = ' ';

    Buffer buf;
    buf << cNumber ( "NUM" )
        << cName   ( "NAME" )
        << cText   ( "TXT" )
        << cAudio  ( "AUD" )
        << cACL    ( "ACL" );

    cNumber.color = xcnone;
    cName.color   = xcnone;
    cText.color   = xcnone;
    cAudio.color  = xcnone;
    cACL.color    = xcnone;

    string tmp;
    uint32 num = 0;
    const LevelDB::mapLEVEL_t::const_iterator end = levelDB.mapLEVEL.end();
    for ( LevelDB::mapLEVEL_t::const_iterator it = levelDB.mapLEVEL.begin(); it != end; it++ ) {
        const Level& lev = it->second;

        if (!filter.name.empty()) {
            tmp = lev.name;
            str::toLower( tmp );
            if (tmp.find( filter.name ) == string::npos)
                continue;
        }

        if (++num > (Page::maxLines * Page::maxPages))
            break;

        buf << '\n'
            << cNumber ( lev.level )
            << cName   ( str::etAlignLeft( lev.namex, cName.width, tmp ))
            << cText   ( lev.greetingText.empty() ? "no" : "yes" )
            << cAudio  ( lev.greetingAudio.empty() ? "no" : "yes" );

        static const int acmax = 5; // max grants/denies per line
        int acnum = 0;

        // process granted privileges
        {
            const PrivilegeSet::IndexName::const_iterator max = lev.privGranted._indexName.end();
            for ( PrivilegeSet::IndexName::const_iterator it = lev.privGranted._indexName.begin(); it != max; it++ ) {
                if (++acnum > acmax)
                    break;
                buf << cACL( "+" + it->second->_name );
            }
        }

        // process denied privileges
        {
            const PrivilegeSet::IndexName::const_iterator max = lev.privDenied._indexName.end();
            for ( PrivilegeSet::IndexName::const_iterator it = lev.privDenied._indexName.begin(); it != max; it++ ) {
                if (++acnum > acmax)
                    break;
                buf << cACL( "-" + it->second->_name );
            }
        }

        if (acnum > acmax)
            buf << cACL( "..." );
    }

    Page::report( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
