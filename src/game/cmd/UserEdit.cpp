#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserEdit::UserEdit()
    : AbstractBuiltin( "useredit" )
{

    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-acl" )
            << ' ' << "[" << xvalue( '+' ) << '|' << xvalue( '-' ) << ']' << xvalue( "PRIVILEGE" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-gtext TEXT" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-gaudio WAVEFILE" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-level LEVEL" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-nadd TEXT" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-ndelete LINE" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-nedit LINE TEXT" )
        << '\n' << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" ) << ' ' << xvalue( "-ninsert LINE TEXT" );

    __descr << "Edit an admin level.";
}

///////////////////////////////////////////////////////////////////////////////

UserEdit::~UserEdit()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserEdit::doExecute( Context& txt )
{
    if (txt._args.size() < 3)
        return PA_USAGE;

    // bail on invalid id
    User& user = lookupUSER( txt._args[1], txt );
    if (user == User::BAD)
        return PA_ERROR;

    if (isHigherLevelError( user, txt ))
        return PA_ERROR;

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

        if (s == "-gtext") {
            str::concatArgs( txt._args, user.greetingText, ++i );
            buf << "modified greeting text: " << xvalue( user.greetingText );
            break;
        }

        if (s == "-gaudio") {
            str::concatArgs( txt._args, user.greetingAudio, ++i );
            buf << "modified greeting audio: " << xvalue( user.greetingAudio );
            break;
        }

        if (s == "-nadd") {
            if (!(user.notes.size() < User::notesMax)) {
                txt._ebuf << "Notes are full: " << (int)user.notes.size() << " lines";
                return PA_ERROR;
            }

            str::concatArgs( txt._args, s, ++i );
            user.notes.push_back( s );
            buf << "added note line " << (int)user.notes.size() << ": " << xvalue( s );
            break;
        }

        if (s == "-ndelete") {
            if (user.notes.empty()) {
                txt._ebuf << "Notes are empty";
                return PA_ERROR;
            }

            s = txt._args[++i];
            int line = atoi( s.c_str() );

            if (line < 1) {
                txt._ebuf << "minimum LINE is 1";
                return PA_ERROR;
            }

            if (line > (int)user.notes.size()) {
                txt._ebuf << "maximum LINE is " << (int)user.notes.size();
                return PA_ERROR;
            }

            vector<string>::iterator it = user.notes.begin() + (line-1);
            s = *it;
            user.notes.erase( it );
            buf << "deleted note line " << line << ": " << xvalue( s );
            break;
        }

        if (s == "-nedit") {
            if (user.notes.empty()) {
                txt._ebuf << "Notes are empty";
                return PA_ERROR;
            }

            s = txt._args[++i];
            int line = atoi( s.c_str() );

            if (line < 1) {
                txt._ebuf << "minimum LINE is 1";
                return PA_ERROR;
            }

            if (line > (int)user.notes.size()) {
                txt._ebuf << "maximum LINE is " << (int)user.notes.size();
                return PA_ERROR;
            }

            str::concatArgs( txt._args, user.notes[line-1], ++i );
            buf << "edited note line " << line << ": " << xvalue( user.notes[line-1] );
            break;
        }

        if (s == "-ninsert") {
            if (!(user.notes.size() < User::notesMax)) {
                txt._ebuf << "Notes are full: " << (int)user.notes.size() << " lines";
                return PA_ERROR;
            }

            s = txt._args[++i];
            int line = atoi( s.c_str() );

            if (line < 1) {
                txt._ebuf << "minimum LINE is 1";
                return PA_ERROR;
            }

            if (!user.notes.empty() && line > (int)user.notes.size()) {
                txt._ebuf << "maximum LINE is " << (int)user.notes.size();
                return PA_ERROR;
            }

            str::concatArgs( txt._args, s, ++i );
            vector<string>::iterator it = user.notes.begin() + (line-1);
            user.notes.insert( it, s );
            buf << "inserted note line " << line << ": " << xvalue( user.notes[line-1] );
            break;
        }

        if (s == "-acl") {
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
                case '+':
                case '-':
                    break;

                default:
                    txt._ebuf << "Invalid privilege prefix: expecting "
                        << xvalue( '+' ) << " or " << xvalue( '-' ) << " or " << xvalue( '!' );
                    return PA_ERROR;
            }

            // allocate new psets as required
            if (!user.privGranted)
                user.privGranted = new PrivilegeSet;
            if (!user.privDenied)
                user.privDenied = new PrivilegeSet;

            switch (s[0]) {
                default:
                case '!':
                    user.privGranted->erase( p );
                    user.privDenied->erase( p ); 
                    buf << "cleared privilege: " << xvalue( p._name );
                    break;

                case '+':
                    user.privGranted->insert( p ); 
                    buf << "added grant: " << xvalue( p._name );
                    break;

                case '-':
                    user.privDenied->insert( p ); 
                    buf << "added deny: " << xvalue( p._name );
                    break;
            }

            PrivilegeSet::canonicalize( *user.privGranted, *user.privDenied );

            // delete empty psets
            if (user.privGranted->empty()) {
                delete user.privGranted;
                user.privGranted = NULL;
            }
            if (user.privDenied->empty()) {
                delete user.privDenied;
                user.privDenied = NULL;
            }
        }
        else if (s == "-level") {
            Level& lev = lookupLEVEL( txt._args[++i], txt );
            if (lev == Level::BAD)
                return PA_ERROR;

            // bail if fake GUID
            if (user.fakeguid) {
                txt._ebuf << xvalue( user.namex ) << " has no GUID.";
                return PA_ERROR;
            }

            user.authLevel = lev.level;

            buf << xvalue( user.namex ) << "'s level set to " << xvalue( lev.level );
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
