#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

UserDB::UserDB()
    : Database      ( "user.db", "guid" )
    , _maxAnonymous ( 16384 )
    , mapGUID       ( _mapGUID )
    , mapBANTIME    ( _mapBANTIME )
    , mapIP         ( _mapIP )
    , mapMAC        ( _mapMAC )
    , mapNAME       ( _mapNAME )
    , mapTIME       ( _mapTIME )
    , maxAnonymous  ( _maxAnonymous )
{
}

///////////////////////////////////////////////////////////////////////////////

UserDB::~UserDB()
{
}

///////////////////////////////////////////////////////////////////////////////

UserDB::BanStatus
UserDB::checkBan( string guid, string ip, string mac, User*& subject, string& detail )
{
    subject = NULL;
    detail.clear();

    const time_t now = time( NULL );
    BanStatus status = BAN_NONE;

    // GUID
    if (guid.length() == 32) {
        string err;
        User &user = fetchByKey( guid, err );

        if (user != User::BAD && user.banned) {
            subject = &user;
            detail = "GUID " + user.guid;

            if (!user.banExpiry)
                return BAN_ACTIVE;

            if (user.banExpiry > now)
                return BAN_ACTIVE;

            status = BAN_LIFTED;
        }
    }

    // MAC
    if (!mac.empty()) {
        const mapMAC_t::iterator end = _mapMAC.upper_bound( mac );
        for ( mapMAC_t::iterator it = _mapMAC.lower_bound( mac ); it != end; it++ ) {
            User &user = *it->second;

            if (!user.banned)
                continue;

            subject = &user;
            detail = "MAC " + user.mac;

            if (!user.banExpiry)
                return BAN_ACTIVE;

            if (user.banExpiry > now)
                return BAN_ACTIVE;

            status = BAN_LIFTED;
        }
    }

    // IP
    if (!ip.empty()) {
        const mapIP_t::iterator end = _mapIP.upper_bound( ip );
        for ( mapIP_t::iterator it = _mapIP.lower_bound( ip ); it != end; it++ ) {
            User &user = *it->second;

            if (!user.banned)
                continue;

            subject = &user;
            detail = "IP " + user.ip;

            if (!user.banExpiry)
                return BAN_ACTIVE;

            else if (user.banExpiry > now)
                return BAN_ACTIVE;

            status = BAN_LIFTED;
        }
    }

    if (status == BAN_LIFTED) {
        unindex( *subject );
        subject->banned = false;
        index( *subject );
    }

    return status;
}

///////////////////////////////////////////////////////////////////////////////

User&
UserDB::fetchByID( const string& id, string& err )
{
    const string::size_type idlen = id.length();
    if (idlen < 8) {
        err = "must be at least 8 chars long";
        return User::BAD;
    }
    if (idlen > 32) {
        err = "exceeds maximum of 32 chars";
        return User::BAD;
    }

    const string::size_type foundpos = 32 - idlen;

    string lid = id;
    str::toLower( lid );

    int count = 0;
    User* found = NULL;
    const mapGUID_t::iterator max = _mapGUID.end();
    for ( mapGUID_t::iterator it = _mapGUID.begin(); it != max; it++ ) {
        User& user = it->second;
        if (user.guid.rfind( lid ) == foundpos) {
            found = &user;
            count++;
        }
    }

    if (count == 0) {
        err = "not found";
        return User::BAD;
    }
    else if (count == 1) {
        return *found;
    }
    else {
        err = "ambiguous";
        return User::BAD;
    }
}

///////////////////////////////////////////////////////////////////////////////

User&
UserDB::fetchByKey( const string& guid, string& err, bool create )
{
    if (guid.length() != 32) {
        err = "length must be 32 chars";
        return User::BAD;
    }

    string key = guid;
    str::toLower( key );

    const mapGUID_t::iterator found = _mapGUID.find( key );
    if (found != _mapGUID.end())
        return found->second;

    if (!create) {
        err = "not found";
        return User::BAD;
    }

    // create
    User& user = _mapGUID[ key ];
    user = User::DEFAULT;  // inherit default values
    user._guid = key;      // assign correct key

    return user;
}

///////////////////////////////////////////////////////////////////////////////

bool
UserDB::fetchByName( const string& name, list<User*>& users, string& err )
{
    if (name.empty()) {
        err = "is zero-length";
        return true;
    }

    users.clear();

    // name lookups are case-insensitive
    string key = name;
    str::toLower( key );

    // Loop through looking for matches
    const mapGUID_t::iterator max = _mapGUID.end();
    for ( mapGUID_t::iterator it = _mapGUID.begin(); it != max; it++ ) {
        string s = it->second.name;
        str::toLower( s );
        if (s.find( key ) != string::npos)
            users.push_back( &it->second );
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::index( User& user )
{
    if (user.banned) {
        bool insert = true;

        const mapBANTIME_t::const_iterator max = _mapBANTIME.upper_bound( user.banTime );
        for ( mapBANTIME_t::const_iterator it = _mapBANTIME.lower_bound( user.banTime ); it != max; it++ ) {
            if (*it->second == user) {
                insert = false;
                break;
            }
        }

        if (insert)
            _mapBANTIME.insert( mapBANTIME_t::value_type( user.banTime, &user ));
    }

    if (!user.ip.empty()) {
        bool insert = true;

        const mapIP_t::const_iterator max = _mapIP.upper_bound( user.ip );
        for ( mapIP_t::const_iterator it = _mapIP.lower_bound( user.ip ); it != max; it++ ) {
            if (*it->second == user) {
                insert = false;
                break;
            }
        }

        if (insert)
            _mapIP.insert( mapIP_t::value_type( user.ip, &user ));
    }

    if (!user.mac.empty()) {
        bool insert = true;

        const mapMAC_t::const_iterator max = _mapMAC.upper_bound( user.mac );
        for ( mapMAC_t::const_iterator it = _mapMAC.lower_bound( user.mac ); it != max; it++ ) {
            if (*it->second == user) {
                insert = false;
                break;
            }
        }

        if (insert)
            _mapMAC.insert( mapMAC_t::value_type( user.mac, &user ));
    }

    if (!user.name.empty()) {
        bool insert = true;

        const mapNAME_t::const_iterator max = _mapNAME.upper_bound( user.name );
        for ( mapNAME_t::const_iterator it = _mapNAME.lower_bound( user.name ); it != max; it++ ) {
            if (*it->second == user) {
                insert = false;
                break;
            }
        }

        if (insert)
            _mapNAME.insert( mapNAME_t::value_type( user.name, &user ));
    }

    {
        bool insert = true;

        const mapTIME_t::const_iterator max = _mapTIME.upper_bound( user.timestamp );
        for ( mapTIME_t::const_iterator it = _mapTIME.lower_bound( user.timestamp ); it != max; it++ ) {
            if (*it->second == user) {
                insert = false;
                break;
            }
        }

        if (insert)
            _mapTIME.insert( mapTIME_t::value_type( user.timestamp, &user ));
    }
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::load( bool merge )
{
    if (!merge) {
        _mapGUID.clear();
        _mapBANTIME.clear();
        _mapIP.clear();
        _mapMAC.clear();
        _mapNAME.clear();
        _mapTIME.clear();
    }

    string filename;
    if (open( false, filename ))
        return;

    logBegin( false, filename );

    // parse DEFAULT user information
    map<string,string> data;
    parseData( data );
    User::DEFAULT.decode( data );

    // parse each record
    while (!_stream.rdstate()) {
        parseData( data );

        map<string,string>::const_iterator it = data.find( _key );
        if (it == data.end())
            continue;

        string err;
        User& user = fetchByKey( it->second, err, true );
        if (user.isNull()) {
            ostringstream msg;
            msg << "WARNING: skipping invalid GUID record: " << it->second << endl;
            trap_Printf( msg.str().c_str() );
            _mapGUID.erase( it->second );
            continue;
        }

        user.decode( data );
        index(user);
    }

    logEnd( _mapGUID.size(), "users" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

uint32
UserDB::migrateAuth( int oldauth, int newauth )
{
    uint32 numMigrated = 0;

    // migrate existing users 
    const mapGUID_t::iterator max = _mapGUID.end();
    for ( mapGUID_t::iterator it = _mapGUID.begin(); it != max; it++ ) {
        User& user = it->second;
        if (user.authLevel != oldauth)
            continue;
    
        user.authLevel = newauth;
        numMigrated++; 
    }

    return numMigrated;
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::purge()
{
    /* NOTE: THIS METHOD IS ONLY SAFE TO CALL AT SDK SHUTDOWN TIME.
     * Loop through (sorted) mapTIME and purge all anonymous records beyond max.
     */
    unsigned int count = 0;
    unsigned int purged = 0;

    const mapTIME_t::reverse_iterator end = _mapTIME.rend();
    for ( mapTIME_t::reverse_iterator it = _mapTIME.rbegin(); it != end; it++ ) {
        User& user = *it->second;

        // Skip non-anon users.
        if (user.authLevel)
            continue;

        // Skip banned users.
        if (user.banned)
            continue;

        count++;
        if (count > _maxAnonymous) {
            _mapGUID.erase( user.guid );
            purged++;
        }
    }

    if (!purged)
        return;

    ostringstream msg;
    msg << "ANONYMOUS USERS PURGED: " << purged << endl;
    trap_Printf( msg.str().c_str() );

}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::remove( User& obj )
{
    unindex( obj );
    _mapGUID.erase( obj._guid );
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::save()
{
    // Open file
    string filename;
    if ( open( true, filename ) )
        return;

    logBegin( true, filename );

    time_t now = time( 0 );
    char fnow[32];
    strftime( fnow, sizeof(fnow), "%c", localtime( &now ));

    // Output header
    _stream
        << "###############################################################################"
        << '\n' << "##"
        << '\n' << "## " << JAYMOD_title << " -- " << _filename
        << '\n' << "## updated: " << fnow
        << '\n' << "## records: " << _mapGUID.size() << "  (bans: " << _mapBANTIME.size() << ')'
        << '\n' << "##"
        << '\n' << "###############################################################################";

    // Output default user
    int recnum = 0;
    User::DEFAULT.encode( _stream, recnum++ );

    // Output users
    const mapGUID_t::iterator max = _mapGUID.end();
    for ( mapGUID_t::iterator it = _mapGUID.begin(); it != max; it++ ) {
        // We don't save fake GUIDs
        if (it->second.fakeguid)
            continue;

        // This exists to clean out unused ban records
        if (it->second.banned == false && it->second.guid.substr(0, 6) == "banloc")
            continue;

        // Write the record
        it->second.encode( _stream, recnum++ );
    }

    _stream << '\n';

    logEnd( _mapGUID.size(), "users" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::unindex( User& user )
{
    if (user.banned) {
        const mapBANTIME_t::iterator max = _mapBANTIME.upper_bound( user.banTime );
        for ( mapBANTIME_t::iterator it = _mapBANTIME.lower_bound( user.banTime ); it != max; it++ ) {
            if (*it->second == user) {
                _mapBANTIME.erase( it );
                break;
            }
        }
    }

    if (!user.ip.empty()) {
        const mapIP_t::iterator max = _mapIP.upper_bound( user.ip );
        for ( mapIP_t::iterator it = _mapIP.lower_bound( user.ip ); it != max; it++ ) {
            if (*it->second == user) {
                _mapIP.erase( it );
                break;
            }
        }
    }

    if (!user.mac.empty()) {
        const mapMAC_t::iterator max = _mapMAC.upper_bound( user.mac );
        for ( mapMAC_t::iterator it = _mapMAC.lower_bound( user.mac ); it != max; it++ ) {
            if (*it->second == user) {
                _mapMAC.erase( it );
                break;
            }
        }
    }

    if (!user.name.empty()) {
        const mapNAME_t::iterator max = _mapNAME.upper_bound( user.name );
        for ( mapNAME_t::iterator it = _mapNAME.lower_bound( user.name ); it != max; it++ ) {
            if (*it->second == user) {
                _mapNAME.erase( it );
                break;
            }
        }
    }

    {
        const mapTIME_t::iterator max = _mapTIME.upper_bound( user.timestamp );
        for ( mapTIME_t::iterator it = _mapTIME.lower_bound( user.timestamp ); it != max; it++ ) {
            if (*it->second == user) {
                _mapTIME.erase( it );
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
UserDB::xpResetAll()
{
    const mapGUID_t::iterator end = _mapGUID.end();
    for ( mapGUID_t::iterator it = _mapGUID.begin(); it != end; it++ )
        it->second.xpReset();
}


///////////////////////////////////////////////////////////////////////////////

UserDB userDB;
User*  connectedUsers[ MAX_CLIENTS ];
