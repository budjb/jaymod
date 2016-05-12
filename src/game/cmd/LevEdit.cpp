#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LevEdit::LevEdit()
    : AbstractBuiltin( "levedit" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" ) << ' ' << xvalue( "-acl" )
            << ' ' << "[" << xvalue( '+' ) << '|' << xvalue( '-' ) << ']' << xvalue( "PRIVILEGE" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" ) << ' ' << xvalue( "-name NAME" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" ) << ' ' << xvalue( "-gtext TEXT" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" ) << ' ' << xvalue( "-gaudio WAVEFILE" );

    __descr << "Edit an admin level.";
}

///////////////////////////////////////////////////////////////////////////////

LevEdit::~LevEdit()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LevEdit::doExecute( Context& txt )
{
    if (txt._args.size() < 3)
        return PA_USAGE;

    // bail on invalid level
    string err;
    const string& name = txt._args[1];
    Level& lev = levelDB.fetchByName( name, err );
    if (lev == Level::BAD) {
        txt._ebuf << "LEVEL " << err << ": " << xvalue( name ) << " .";
        return PA_ERROR;
    }

    // parse options
    string shead = "-" + _name;
    str::toUpper( shead );

    Buffer buf;
    buf << _name << ": ";

    // parse final pair
    const vector<string>::size_type max = txt._args.size();
    for ( vector<string>::size_type i = 2; i < max; i++ ) {
        // pairs of args are expected
        if ((max - i) < 2)
            return PA_USAGE;

        string s = txt._args[i];
        str::toLower( s );

        if (s == "-name" ) {
            str::concatArgs( txt._args, lev.namex, ++i );
            lev.name = lev.namex;
            str::etDecolorize( lev.name );
            buf << "modified name: " << xvalue( lev.namex );
            break;
        }
        else if (s == "-gtext") {
            str::concatArgs( txt._args, lev.greetingText, ++i );
            buf << "modified greeting text: " << xvalue( lev.greetingText );
            break;
        }
        else if (s == "-gaudio") {
            str::concatArgs( txt._args, lev.greetingAudio, ++i );
            buf << "modified greeting audio: " << xvalue( lev.greetingAudio );
            break;
        }
        else if (s == "-acl") {
            s = txt._args[++i];
            if (s.length() < 2) {
                txt._ebuf << "Invalid privilege: " << xvalue( txt._args[i-1] );
                return PA_ERROR;
            }
            str::toLower( s );
            const string pname = s.substr( 1 );

            const PrivilegeSet::IndexName::const_iterator found = PrivilegeSet::REGISTRY._indexName.find( pname );
            if (found == PrivilegeSet::REGISTRY._indexName.end()) {
                txt._ebuf << "Invalid privilege: " << xvalue( pname );
                return PA_ERROR;
            }
            const Privilege& p = *found->second;

            switch (s[0]) {
                case '!':
                    lev.privGranted.erase( p );
                    lev.privDenied.erase( p );
                    buf << "cleared privilege: " << xvalue( p._name );
                    break;

                case '+':
                    lev.privGranted.insert( p );
                    buf << "added grant: " << xvalue( p._name );
                    break;

                case '-':
                    lev.privDenied.insert( p );
                    buf << "added deny: " << xvalue( p._name );
                    break;

                default:
                    txt._ebuf << "Invalid privilege prefix: expecting "
                        << xvalue( '+' ) << " or " << xvalue( '-' ) << " or " << xvalue( '!' );
                    return PA_ERROR;
            }

            PrivilegeSet::canonicalize( lev.privGranted, lev.privDenied );
        }
        else {
            return PA_USAGE;
        }
    }

    printChat( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
