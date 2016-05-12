#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace {
    void convertFlagAndAppend( const string&, string& );
}  // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

PrivilegeSet::PrivilegeSet()
    : _handleSet ( __handleSet )
    , _indexName ( __indexName )
{
}

///////////////////////////////////////////////////////////////////////////////

PrivilegeSet::~PrivilegeSet()
{
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::canonicalize( PrivilegeSet& granted, PrivilegeSet& denied )
{
    if (denied.__handleSet.find( &priv::pseudo::all ) != denied.__handleSet.end()) {
        granted.clear();
        denied.clear();
        denied.insert( priv::pseudo::all );
        return;
    }

    static const uint32
        CLEAR_BEHAVIORS = 0x01,
        CLEAR_COMMANDS  = 0x02;

    uint32 clear = 0;
    if (denied.__handleSet.find( &priv::pseudo::behaviors ) != denied.__handleSet.end())
        clear |= CLEAR_BEHAVIORS;
    if (denied.__handleSet.find( &priv::pseudo::commands ) != denied.__handleSet.end())
        clear |= CLEAR_COMMANDS;

    if (granted.__handleSet.find( &priv::pseudo::all ) != granted.__handleSet.end()) {
        granted.clear();
        granted.insert( priv::pseudo::all );
    }

    // remove denied from granted
    // remove any grant-always from denied
    {
        list<const Privilege*> rmlist;
        const HandleSet::iterator max = denied.__handleSet.end();
        for ( HandleSet::iterator it = denied.__handleSet.begin(); it != max; it++ ) {
            const Privilege& p = **it;

            granted.erase( p );

            if (p._grantAlways) {
                rmlist.push_back( &p );
                continue;
            }

            if (!clear)
                continue;

            switch (p._type) {
                case Privilege::TYPE_BEHAVIORAL:
                    if (clear & CLEAR_BEHAVIORS) {
                        rmlist.push_back( &p );
                        granted.erase( p );
                    }
                    break;

                case Privilege::TYPE_COMMAND:
                case Privilege::TYPE_CUSTOM:
                    if (clear & CLEAR_COMMANDS) {
                        rmlist.push_back( &p );
                        granted.erase( p );
                    }
                    break;

                default:
                    break;
            }
        }

        denied.erase( rmlist );
    }

    if (granted.__handleSet.find( &priv::pseudo::behaviors ) != granted.__handleSet.end())
        clear |= CLEAR_BEHAVIORS;
    if (granted.__handleSet.find( &priv::pseudo::commands ) != granted.__handleSet.end())
        clear |= CLEAR_COMMANDS;

    // remove any grant-always from granted
    {
        list<const Privilege*> rmlist;
        const HandleSet::iterator max = granted.__handleSet.end();
        for ( HandleSet::iterator it = granted.__handleSet.begin(); it != max; it++ ) { 
            const Privilege& p = **it;
            if (p._grantAlways) {
               rmlist.push_back( &p );
               continue;
            }

            if (!clear)
                continue;

            switch (p._type) {
                case Privilege::TYPE_BEHAVIORAL:
                    if (clear & CLEAR_BEHAVIORS)
                        rmlist.push_back( &p );
                    break;

                case Privilege::TYPE_COMMAND:
                case Privilege::TYPE_CUSTOM:
                    if (clear & CLEAR_COMMANDS)
                        rmlist.push_back( &p );
                    break;

                default:
                    break;
            }
        }

        granted.erase( rmlist );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::clear()
{
    __handleSet.clear();
    __indexName.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool
PrivilegeSet::contains( const Privilege& priv ) const
{
    return (__handleSet.find( &priv ) != __handleSet.end());
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::decode( PrivilegeSet& granted, PrivilegeSet& denied, const string& acltext, const string& flags )
{
    /* flags is for backwards compatibility of old style authorization flags.
     * We will support upwards compatibility by reading flags and converting them to ACL style.
     */
    string text = acltext;
    convertFlagAndAppend( flags, text );

    enum Mode { DELIM, VALUE };
    Mode mode = DELIM;

    granted.clear();
    denied.clear();

    list<string> acl;

    string token;
    const string::size_type max = text.length();
    for ( string::size_type i = 0; i < max; i++) {
        const char c = text[i];
        switch (mode) {
        case DELIM:
            switch (c) {
            case '\t':  // TAB
            case ' ':   // SPACE
            case ',':   // DELIMITER
                break;

            default:
                mode = VALUE;
                token += c;
                break;
            }
            break;

        case VALUE:
            switch (c) {
            case '\t':  // TAB
            case ' ':   // SPACE
            case ',':   // DELIMITER
                mode = DELIM;
                if (!token.empty()) {
                    acl.push_back( token );
                    token.clear();
                }
                break;

            default:
                token += c;
                break;
            }
            break;

        default:
            break;
        }
    }

    if (!token.empty()) {
        acl.push_back( token );
        token.clear();
    }

    // process acl
    {
        const list<string>::iterator max = acl.end();
        for ( list<string>::iterator it = acl.begin(); it != max; it++ ) {
            const string& s = *it;

            // skip if too small
            if (s.length() < 2)
                continue;

            bool grant;
            switch (s[0]) {
                case '-':  grant = false;  break;
                case '+':  grant = true;   break;

                default:
                    continue;
            }

            // trim modifier
            string name = s.substr( 1 );
            str::toLower( name );

            const IndexName::const_iterator found = REGISTRY._indexName.find( name );
            if (found == REGISTRY._indexName.end())
                continue;

            const Privilege& p = *found->second;
            if (grant)
                granted.insert( p );
            else
                denied.insert( p );
        }
    }

    canonicalize( granted, denied );
}

///////////////////////////////////////////////////////////////////////////////

bool
PrivilegeSet::empty() const
{
    return __handleSet.empty();
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::encode( const PrivilegeSet& granted, const PrivilegeSet& denied, ostream& out )
{
    int num = 0;

    // process granted privileges
    {
        const IndexName::const_iterator max = granted._indexName.end();
        for ( IndexName::const_iterator it = granted._indexName.begin(); it != max; it++ ) {
            const Privilege& p = *it->second;

            if (num++ == 0)
                out << '+' << p._name;
            else
                out << " +" << p._name;
        }
    }

    // process denied privileges
    {
        const IndexName::const_iterator max = denied._indexName.end();
        for ( IndexName::const_iterator it = denied._indexName.begin(); it != max; it++ ) {
            const Privilege& p = *it->second;

            if (num++ == 0)
                out << '-' << p._name;
            else
                out << " -" << p._name;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::encode( const PrivilegeSet* granted, const PrivilegeSet* denied, ostream& out )
{
    if (granted && denied) {
        encode( *granted, *denied, out );
    }
    else if (granted && !denied) {
        PrivilegeSet placebo;
        encode( *granted, placebo, out );
    }
    else if (!granted && denied) {
        PrivilegeSet placebo;
        encode( placebo, *denied, out );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::erase( const Privilege& priv )
{
    __handleSet.erase( &priv );
    __indexName.erase( priv._nameLower );
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::erase( const list<const Privilege*>& plist )
{
    const list<const Privilege*>::const_iterator max = plist.end();
    for ( list<const Privilege*>::const_iterator it = plist.begin(); it != max; it++) {
        const Privilege& priv = **it;
        __handleSet.erase( &priv );
        __indexName.erase( priv._nameLower );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
PrivilegeSet::insert( const Privilege& priv )
{
    __handleSet.insert( &priv );
    __indexName[priv._nameLower] = &priv;
}

///////////////////////////////////////////////////////////////////////////////

PrivilegeSet&
PrivilegeSet::operator=( const PrivilegeSet& ref )
{
    __handleSet = ref.__handleSet;
    __indexName = ref.__indexName;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void
convertFlagAndAppend( const string& flags, string& out )
{
    bool permit = true;
    ostringstream oss;

    const string::size_type max = flags.length();
    for ( string::size_type i = 0; i < max; i++ ) {
        vector<string> privs;

        const char c = flags[i];
        switch (c) {
            default:
                continue;

            case '+':
                permit = true;
                continue;

            case '-':
                permit = false;
                continue;

            case '*':  privs.push_back( "@all"                ); break;
            case '1':  privs.push_back( "b/voteimmunity"      ); break; 
            case '2':  privs.push_back( "b/censorimmunity"    ); break; 
            case '3':  privs.push_back( "b/commandsilent"     ); break; 
            case '4':  privs.push_back( "b/specchat"          ); break; 
            case '5':  privs.push_back( "b/balanceimmunity"   ); break; 
            case '6':  privs.push_back( "b/reasonnone"        ); break; 
            case '7':  privs.push_back( "b/voteany"           ); break; 
            case '8':  privs.push_back( "b/banpermanent"      ); break; 
            case '9':  privs.push_back( "b/commandchat"       ); break; 
            case 'A':  privs.push_back( "c/about"             ); break; 
            case 'C':  privs.push_back( "c/time"              ); break; 
            case 'E':  privs.push_back( "c/seen"              ); break; 
            case 'G':  privs.push_back( "c/dbload    "        ); break; 
            case 'H':  privs.push_back( "c/shake"             ); break; 
            case 'I':  privs.push_back( "c/finger"            ); break; 
            case 'N':  privs.push_back( "c/rename"            ); break; 
            case 'P':  privs.push_back( "c/spec999"           ); break; 
            case 'R':  privs.push_back( "c/resetxp"           ); break; 
            case 'S':  privs.push_back( "c/shuffle"           ); break; 
            case 'U':  privs.push_back( "c/smite"             ); break; 
            case 'W':  privs.push_back( "c/news"              ); break; 
            case 'X':  privs.push_back( "c/resetmyxp"         ); break; 
            case 'Z':  privs.push_back( "c/pause"             ); break; 
            case '_':  privs.push_back( "c/status"            ); break; 
            case 'a':  privs.push_back( "c/admintest"         ); break; 
            case 'c':  privs.push_back( "c/cancelvote"        ); break; 
            case 'e':  privs.push_back( "c/spec"              ); break; 
            case 'g':  privs.push_back( "c/ftime"             ); break; 
            case 'h':  privs.push_back( "c/help"              ); break; 
            case 'k':  privs.push_back( "c/kick"              ); break; 
            case 'n':  privs.push_back( "c/nextmap"           ); break; 
            case 'p':  privs.push_back( "c/putteam"           ); break; 
            case 's':  privs.push_back( "c/setlevel"          ); break; 
            case 'u':  privs.push_back( "c/uptime"            ); break; 
            case 'v':  privs.push_back( "c/revive"            ); break; 
            case 'w':  privs.push_back( "c/swap"              ); break; 
            case 'x':  privs.push_back( "c/lol"               ); break; 
            case 'y':  privs.push_back( "c/crybaby"           ); break; 

            case 'B':  privs.push_back( "c/baninfo"           );
                       privs.push_back( "c/banlist"           ); break; 

            case 'K':  privs.push_back( "c/lock"              );
                       privs.push_back( "c/unlock"            ); break;

            case 'L':  privs.push_back( "c/flinga"            );
                       privs.push_back( "c/launcha"           );
                       privs.push_back( "c/splata"            );
                       privs.push_back( "c/throwa"            ); break;

            case 'M':  privs.push_back( "c/crazygravity"      );
                       privs.push_back( "c/panzerwar"         );
                       privs.push_back( "c/sniperwar"         ); break;

            case 'd':  privs.push_back( "c/disorient"         );
                       privs.push_back( "c/orient"            ); break;

            case 'b':  privs.push_back( "c/ban"               );
                       privs.push_back( "c/unban"             ); break;

            case 'i':  privs.push_back( "c/listplayers"       );
                       privs.push_back( "c/lslevels"          ); break;

            case 'l':  privs.push_back( "c/fling"             );
                       privs.push_back( "c/launch"            );
                       privs.push_back( "c/slap"              );
                       privs.push_back( "c/splat"             );
                       privs.push_back( "c/throw"             ); break;

            case 'm':  privs.push_back( "c/mute"              );
                       privs.push_back( "c/unmute"            ); break;

            case 'r':  privs.push_back( "c/reset"             );
                       privs.push_back( "c/restart"           ); break;

            case 'z':  privs.push_back( "c/glow"              );
                       privs.push_back( "c/pants"             );
                       privs.push_back( "c/pip"               );
                       privs.push_back( "c/pop"               );
                       privs.push_back( "c/unpause"           ); break; 
        }

        const vector<string>::size_type jmax = privs.size();
        for ( vector<string>::size_type j = 0; j < jmax; j++ )
            oss << ' ' << (permit ? '+' : '-') << privs[j];
    }

    out += oss.str();
}

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous
