#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

LevelDB::LevelDB()
    : Database ( "level.db", "level" )
    , mapLEVEL ( _mapLEVEL )
{
}

///////////////////////////////////////////////////////////////////////////////

LevelDB::~LevelDB()
{
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByKey( const string& key, string& err, bool create )
{
    return fetchByKey( toKey( key ), err, create );
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByKey( int key, string& err, bool create )
{
    if (key < Level::NUM_MIN) {
        ostringstream oss;
        oss << "minimum is " << Level::NUM_MIN;
        err = oss.str();
        return Level::BAD;
    }

    if (key > Level::NUM_MAX) {
        ostringstream oss;
        oss << "maximum is " << Level::NUM_MAX;
        err = oss.str();
        return Level::BAD;
    }

    const mapLEVEL_t::iterator found = _mapLEVEL.find( key );
    if (found != _mapLEVEL.end())
        return found->second;

    if (!create) {
        err = "not found";
        return Level::BAD;
    }

    // create
    Level& lev = _mapLEVEL[ key ];
    lev._level      = key;
    lev.privGranted = Level::DEFAULT.privGranted;
    lev.privDenied  = Level::DEFAULT.privDenied ;

    return lev;
}

///////////////////////////////////////////////////////////////////////////////

Level&
LevelDB::fetchByName( const string& name, string& err )
{
    if (name.empty()) {
        err = "is empty";
        return Level::BAD;
    }

    string lname = name;
    str::toLower( lname );

    if (str::isIndex( lname ))
        return fetchByKey( name, err );

    Level* found = NULL;
    int count = 0;
    const mapLEVEL_t::iterator max = _mapLEVEL.end();
    for ( mapLEVEL_t::iterator it = _mapLEVEL.begin(); it != max; it++ ) {
        Level& lev = it->second;
        string tmp = lev.name;
        str::toLower( tmp );
        if (tmp.find( lname ) != string::npos) {
            found = &lev;
            count++;
        }
    }

    if (count == 0) {
        err = "not found";
        return Level::BAD;
    }
    else if (count == 1) {
        return *found;
    }
    else {
        err = "ambiguous";
        return Level::BAD;
    }
}

///////////////////////////////////////////////////////////////////////////////

void
LevelDB::load()
{
    _mapLEVEL.clear();

    {
        // setup DEFAULT level
        Level& level = Level::DEFAULT;
        level._level = 0;
        level.name = "default";
        level.namex = level.name;
        level.privGranted.insert( cmd::builtins::listPlayers._privilege );
        level.privGranted.insert( cmd::builtins::resetmyXp._privilege );

        _mapLEVEL[ level._level ] = level;
    }

    // Open file
    string filename;
    if (open( false, filename ))
        return;

    logBegin( false, filename );

    // parse DEFAULT user information
    map<string,string> data;

    // Parse each record
    while (!_stream.rdstate()) {
        parseData( data );

        map<string,string>::const_iterator it = data.find( _key );
        if (it == data.end())
            continue;

        const int reckey = toKey( it->second );

        string err;
        Level& level = fetchByKey( reckey, err, true );
        if (level == Level::BAD) {
            ostringstream msg;
            msg << "WARNING: skipping invalid LEVEL record: " << it->second << endl;
            trap_Printf( msg.str().c_str() );
            _mapLEVEL.erase( reckey );
            continue;
        }

        level.decode( data );
    }

    logEnd( _mapLEVEL.size(), "levels" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

uint32
LevelDB::remove( Level& obj, const Level& migrate )
{
    const uint32 result = userDB.migrateAuth( obj.level, migrate.level );
    _mapLEVEL.erase( obj.level );
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void
LevelDB::save()
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
    _stream << "###############################################################################"
        << '\n' << "##"
        << '\n' << "## " << JAYMOD_title << " -- " << _filename
        << '\n' << "## updated: " << fnow
        << '\n' << "## levels:  " << _mapLEVEL.size()
        << '\n' << "##"
        << '\n' << "###############################################################################";

    // Output users
    int recnum = 0;
    const mapLEVEL_t::iterator max = _mapLEVEL.end();
    for ( mapLEVEL_t::iterator it = _mapLEVEL.begin(); it != max; it++ )
        it->second.encode( _stream, recnum++ );

    _stream << '\n';

    logEnd( _mapLEVEL.size(), "levels" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

int
LevelDB::toKey( const string& key )
{
    int ikey = -1;
    istringstream( key ) >> ikey;
    return ikey;
}

///////////////////////////////////////////////////////////////////////////////

LevelDB levelDB;
