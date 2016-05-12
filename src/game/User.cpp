#include <bgame/impl.h>

#undef JAYMOD_USERDB_DEBUG

///////////////////////////////////////////////////////////////////////////////

User::User( )
    : guid          ( _guid )
    , fakeguid      ( false )
    , timestamp     ( 0 )
    , authLevel     ( 0 )
    , privGranted   ( NULL )
    , privDenied    ( NULL )
    , muted         ( false )
    , muteTime      ( 0 )
    , muteExpiry    ( 0 )
    , banned        ( false )
    , banTime       ( 0 )
    , banExpiry     ( 0 )
{
    memset( xpSkills, 0, sizeof(xpSkills) );
}

///////////////////////////////////////////////////////////////////////////////

User::User( bool console )
    : guid          ( _guid )
    , fakeguid      ( false )
    , timestamp     ( 0 )
    , authLevel     ( 0 )
    , privGranted   ( NULL )
    , privDenied    ( NULL )
    , muted         ( false )
    , muteTime      ( 0 )
    , muteExpiry    ( 0 )
    , banned        ( false )
    , banTime       ( 0 )
    , banExpiry     ( 0 )
{
    memset( xpSkills, 0, sizeof(xpSkills) );

    if (console) {
        authLevel = Level::NUM_MAX;

        namex = "^1CONSOLE";
        name = namex;
        str::etDecolorize( name );
    }
}

///////////////////////////////////////////////////////////////////////////////

User::User( const User& user )
    : guid        ( _guid )
    , privGranted ( NULL )
    , privDenied  ( NULL )
{
    operator=( user );
}

///////////////////////////////////////////////////////////////////////////////

User::~User()
{
    if (privGranted)
        delete privGranted;

    if (privDenied)
        delete privDenied;
}

///////////////////////////////////////////////////////////////////////////////

void
User::canonicalizePrivileges()
{
    PrivilegeSet granted;
    if (privGranted)
        granted = *privGranted;

    PrivilegeSet denied;
    if (privDenied)
        denied = *privDenied;

    PrivilegeSet::canonicalize( granted, denied );

    if (granted.empty()) {
        if (privGranted) {
            delete privGranted;
            privGranted = NULL;
        }
    }
    else {
        if (!privGranted)
            privGranted = new PrivilegeSet;
        *privGranted = granted;
    }

    if (denied.empty()) {
        if (privDenied) {
            delete privDenied;
            privDenied = NULL;
        }
    }
    else {
        if (!privDenied)
            privDenied = new PrivilegeSet;
        *privDenied = denied;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
User::decode( map<string,string>& data )
{
    stringstream ss;
    ss << data["timestamp"];
    ss >> timestamp;
    if (timestamp < 0)
        timestamp = 0;

    ip = data["ip"];
    int len = ip.length();
    if (len < 1 || len > 15)
        ip = "";

    mac = data["mac"];
    if (mac.length() != 17)
        mac = "";

    name = data["name"];
    namex = data["namex"];

    if (name.length() >= MAX_NETNAME)
        name.resize( MAX_NETNAME-1 );
    if (namex.length() >= MAX_NETNAME)
        name.resize( MAX_NETNAME-1 );

    // namex has priority
    if (namex.empty()) 
        namex = name;
    else 
        name = namex;

    str::etDecolorize( name );

    greetingText = data["greetingtext"];
    greetingAudio = data["greetingaudio"];

    ss.str("");
    ss.clear();
    ss << data["authlevel"];
    ss >> authLevel;

    if (authLevel < 0)
        authLevel = 0;
    else if (authLevel > Level::NUM_MAX)
        authLevel = 0;

    // decode privileges
    {
        if (privGranted)
            privGranted->clear();
        else
            privGranted = new PrivilegeSet;

        if (privDenied)
            privDenied->clear();
        else
            privDenied = new PrivilegeSet;

        // MIKE TODO: for 2.1.7 release we can remove concat'd PRIVILEGES 
        PrivilegeSet::decode( *privGranted, *privDenied, data["acl"] + data["privileges"], data["authflags"] );

        if (privGranted->_handleSet.empty()) {
            delete privGranted;
            privGranted = NULL;
        }

        if (privDenied->_handleSet.empty()) {
            delete privDenied;
            privDenied = NULL;
        }
    }

    string& enc = data["xpskills"];
    if ( enc.length() ) {
        unsigned long crc = base64::crc32( guid.c_str(), guid.length() );
        char buf[ sizeof(xpSkills) + sizeof(crc) + 1]; // for some reason base64_decode requires +1

        // size must match exactly in order to continue XP decoding
        int nbytes = base64::decode( (const unsigned char*)enc.c_str(), (unsigned char*)buf, sizeof(buf) );
        if ( nbytes == sizeof(buf)-1 ) {
            scramble( buf, sizeof(buf)-1 );

            unsigned long sig = *(unsigned long*)(buf + sizeof(xpSkills));
            if (sig == crc)
                memcpy( xpSkills, buf, sizeof(xpSkills) );
        }
    }

    ss.str("");
    ss.clear();
    ss << data["muted"];
    ss >> muted;

    if (muted) {
        ss.str("");
        ss.clear();
        ss << data["mutetime"];
        ss >> muteTime;
        if (muteTime < 0)
            muteTime = 0;

        ss.str("");
        ss.clear();
        ss << data["muteexpiry"];
        ss >> muteExpiry;
        if (muteExpiry < 0)
            muteExpiry = 0;

        muteReason = data["mutereason"];

        muteAuthority = data["muteauthority"];
        muteAuthorityx = data["muteauthorityx"];

        if (muteAuthority.length() >= MAX_NETNAME)
            muteAuthority.resize( MAX_NETNAME-1 );
        if (muteAuthorityx.length() >= MAX_NETNAME)
            muteAuthorityx.resize( MAX_NETNAME-1 );

        if (muteAuthorityx.empty())
            muteAuthorityx = muteAuthority;
        else
            muteAuthority = muteAuthorityx;

        str::etDecolorize( muteAuthority );
    }

    ss.str("");
    ss.clear();
    ss << data["banned"];
    ss >> banned;

    if (banned) {
        ss.str("");
        ss.clear();
        ss << data["bantime"];
        ss >> banTime;
        if (banTime < 0)
            banTime = 0;

        ss.str("");
        ss.clear();
        ss << data["banexpiry"];
        ss >> banExpiry;
        if (banExpiry < 0)
            banExpiry = 0;

        banReason = data["banreason"];

        banAuthority = data["banauthority"];
        banAuthorityx = data["banauthorityx"];

        if (banAuthority.length() >= MAX_NETNAME)
            banAuthority.resize( MAX_NETNAME-1 );
        if (banAuthorityx.length() >= MAX_NETNAME)
            banAuthorityx.resize( MAX_NETNAME-1 );

        if (banAuthorityx.empty())
            banAuthorityx = banAuthority;
        else
            banAuthority = banAuthorityx;

        str::etDecolorize( banAuthority );

        // automatic expiry
        if (banExpiry && banExpiry <= time( NULL ))
            banned = false;
    }

    vector<string>::size_type vss = 0;

    ss.str("");
    ss.clear();
    ss << data["notes"];
    ss >> vss;

    if (vss > notesMax)
        vss = notesMax;

    notes.clear();
    for (vector<string>::size_type i = 0; i < vss; i++) {
        ostringstream oss;
        oss << "note." << (int)(i+1);

        ss.str("");
        ss.clear();
        ss << data[ oss.str() ];

        notes.push_back( ss.str() );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
User::encode( ostream& out, int recnum )
{
    out << '\n';
    if (recnum > 0)
        out << '\n' << "###############################################################################" << '\n';

    if (*this == DEFAULT) {
        out << '\n' << "## DEFAULT VALUES FOR NEW USERS -- IF PRESENT, MUST PRECEDE GUID RECORDS";

        out << '\n' << "authLevel = " << authLevel;

        if (privGranted || privDenied) {
            out << '\n' << "acl = ";
            PrivilegeSet::encode( privGranted, privDenied, out );
        }

        return;
    }

    out << '\n' << "guid = " << guid;

    unsigned long crc = base64::crc32( guid.c_str(), guid.length() );
#if defined( JAYMOD_USERDB_DEBUG )
    out << '\n' << "# guid CRC-32 = " << hex << crc << dec;
#endif // JAYMOD_USERDB_DEBUG

    char ftbuf[32];
    char ftbuf2[32];
    strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &timestamp ));

    out << '\n' << "timestamp = " << timestamp << " # " << ftbuf;
    out << '\n' << "ip = "        << ip;
    out << '\n' << "mac = "       << mac;
    out << '\n' << "name = "      << name;
    out << '\n' << "namex = "     << namex;
    out << '\n' << "authLevel = " << authLevel;

    if (privGranted || privDenied) {
        out << '\n' << "acl = ";
        PrivilegeSet::encode( privGranted, privDenied, out );
    }

#if defined( JAYMOD_USERDB_DEBUG )
    out << '\n' << "#authEffective = " << authority.effective;
    out << '\n' << "#authPermit    = " << authority.permit;
    out << '\n' << "#authDeny      = " << authority.deny;
#endif // JAYMOD_USERDB_DEBUG

    out << '\n' << "greetingText = "  << greetingText;
    out << '\n' << "greetingAudio = " << greetingAudio;

#if defined( JAYMOD_USERDB_DEBUG )
    out << "#xpSkills = " << xpSkills[0]
        << " " << xpSkills[1]
        << " " << xpSkills[2]
        << " " << xpSkills[3]
        << " " << xpSkills[4]
        << " " << xpSkills[5]
        << " " << xpSkills[6]
        << '\n';
#endif // JAYMOD_USERDB_DEBUG

    char data[ sizeof(xpSkills) + sizeof(crc) ];
    memcpy( data, xpSkills, sizeof(xpSkills) );
    memcpy( data + sizeof(xpSkills), &crc, sizeof(crc) );

    scramble( data, sizeof(data) );

    char enc[ sizeof(data) * 4 / 3 + 5 ]; // extra 1 for string-term
    base64::encode( (const unsigned char*)data, sizeof(data), enc, sizeof(enc) );
    out << '\n' << "xpSkills = " << enc;

    if (muted) {
        strftime( ftbuf,  sizeof(ftbuf),  "%c", localtime( &muteTime ));
        if (muteExpiry == 0)
            strcpy( ftbuf2, "PERMANENT" );
        else
            strftime( ftbuf2, sizeof(ftbuf2), "%c", localtime( &muteExpiry ));

        out << '\n' << "muted = "          << muted;
        out << '\n' << "muteTime = "       << muteTime       << " # " << ftbuf;
        out << '\n' << "muteExpiry = "     << muteExpiry     << " # " << ftbuf2;
        out << '\n' << "muteReason = "     << muteReason;
        out << '\n' << "muteAuthority = "  << muteAuthority;
        out << '\n' << "muteAuthorityx = " << muteAuthorityx;
    }

    if (banned) {
        strftime( ftbuf,  sizeof(ftbuf),  "%c", localtime( &banTime ));
        if (banExpiry == 0)
            strcpy( ftbuf2, "PERMANENT" );
        else
            strftime( ftbuf2, sizeof(ftbuf2), "%c", localtime( &banExpiry ));

        out << '\n' << "banned = "        << banned;
        out << '\n' << "banTime = "       << banTime       << " # " << ftbuf;
        out << '\n' << "banExpiry = "     << banExpiry     << " # " << ftbuf2;
        out << '\n' << "banReason = "     << banReason;
        out << '\n' << "banAuthority = "  << banAuthority;
        out << '\n' << "banAuthorityx = " << banAuthorityx;
    }

    if (!notes.empty()) {
        const vector<string>::size_type max = notes.size();
        out << '\n' << "notes = " << max;

        for (vector<string>::size_type i = 0; i < max; i++)
            out << '\n' << "note." << (i+1) << " = " << notes[i];
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
User::hasPrivilege( const Privilege& priv ) const
{
    if (priv._grantAlways)
        return true;

    if (this == &User::CONSOLE)
        return true;

    // cache type-specific pseudo priv
    const Privilege* privForPseudo;
    switch (priv._type) {
        case Privilege::TYPE_BEHAVIORAL:
            privForPseudo = &priv::pseudo::behaviors;
            break;

        case Privilege::TYPE_COMMAND:
        case Privilege::TYPE_CUSTOM:
            privForPseudo = &priv::pseudo::commands;
            break;

        default:
            privForPseudo = NULL;
            break;
    }

    string err;
    Level& lev = levelDB.fetchByKey( authLevel, err );

    if (lev != Level::BAD) {
        if (lev.privDenied.contains( priv::pseudo::all ))
            return false;
        if (privForPseudo && lev.privDenied.contains( *privForPseudo ))
            return false;
        if (lev.privDenied.contains( priv ))
            return false;
    }

    if (privDenied) {
        if (privDenied->contains( priv::pseudo::all ))
            return false;
        if (privForPseudo && privDenied->contains( *privForPseudo ))
            return false;
        if (privDenied->contains( priv ))
            return false;
    }

    if (lev != Level::BAD) {
        if (lev.privGranted.contains( priv::pseudo::all ))
            return true;
        if (privForPseudo && lev.privGranted.contains( *privForPseudo ))
            return true;
        if (lev.privGranted.contains( priv ))
            return true;
    }

    if (privGranted) {
        if (privGranted->contains( priv::pseudo::all ))
            return true;
        if (privForPseudo && privGranted->contains( *privForPseudo ))
            return true;
        if (privGranted->contains( priv ))
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
User::isNull() const
{
    return this == &BAD;
}

///////////////////////////////////////////////////////////////////////////////

bool
User::operator==( const User& user ) const
{
    return this == &user;
}

///////////////////////////////////////////////////////////////////////////////

bool
User::operator!=( const User& user ) const
{
    return this != &user;
}

///////////////////////////////////////////////////////////////////////////////

User&
User::operator=( const User& ref )
{
    _guid         = ref._guid;
    fakeguid      = ref.fakeguid;
    timestamp     = ref.timestamp;
    ip            = ref.ip;
    mac           = ref.mac;
    name          = ref.name;
    namex         = ref.namex;
    greetingText  = ref.greetingText;
    greetingAudio = ref.greetingAudio;
    authLevel     = ref.authLevel;

    if (ref.privGranted) {
        if (!privGranted)
            privGranted = new PrivilegeSet;
        *privGranted = *ref.privGranted;
    }
    else {
        if (privGranted) {
            delete privGranted;
            privGranted = NULL;
        }
    }

    if (ref.privDenied) {
        if (!privDenied)
            privDenied = new PrivilegeSet;
        *privDenied = *ref.privDenied;
    }  
    else {
        if (privDenied) {
            delete privDenied;
            privDenied = NULL;
        }
    }

    memcpy( xpSkills, ref.xpSkills, sizeof(xpSkills) );

    muted          = ref.muted;
    muteTime       = ref.muteTime;
    muteExpiry     = ref.muteExpiry;
    muteReason     = ref.muteReason;
    muteAuthority  = ref.muteAuthority;
    muteAuthorityx = ref.muteAuthorityx;

    banned        = ref.banned;
    banTime       = ref.banTime;
    banExpiry     = ref.banExpiry;
    banReason     = ref.banReason;
    banAuthority  = ref.banAuthority;
    banAuthorityx = ref.banAuthorityx;

    notes = ref.notes;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void
User::scramble( char* data, int size )
{
    // ring buffer of random data for xor'ing
    static const unsigned char ring[] = {
       0x53, 0x21, 0x97, 0xB4, 0xE6, 0x00, 0xE9, 0x23,
       0xEA, 0x1F, 0xDF, 0x7D, 0x46, 0xC9, 0x85, 0xDC,
       0xFE, 0xDE, 0x23, 0x52, 0x67, 0x9F, 0x31, 0x09,
       0x3E, 0x0C, 0xC3, 0x5E, 0xF2, 0xA9, 0x08, 0x9C,
       0x0C, 0xE6, 0x22, 0xA4, 0x02, 0x9D, 0xB4, 0x61,
       0xAA, 0x77, 0xBB, 0xFA, 0x97, 0xB9, 0x91, 0x19,
       0x44, 0x65, 0x49, 0x66, 0xAD, 0x09, 0x95, 0xAA,
       0x69, 0x4E, 0x8E, 0x3C, 0x21, 0xC4, 0x48, 0xC1,
    };

    char* const pmax = data + size;
    const char* const rmax = (char *)ring + sizeof(ring);

    const char* r = (char *)ring;
    for (char* p = data; p < pmax; p++) {
        *p ^= *r++;

        if (r >= rmax)
            r = (char *)ring;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
User::reset()
{
    string guidBak = _guid;
    *this = User::DEFAULT;
    _guid = guidBak;
}

///////////////////////////////////////////////////////////////////////////////

void
User::xpReset()
{
    memset( xpSkills, 0, sizeof(xpSkills) );
}

///////////////////////////////////////////////////////////////////////////////

User User::BAD;
User User::DEFAULT;
User User::CONSOLE( true );

const vector<string>::size_type User::notesMax = 9;
