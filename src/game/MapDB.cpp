#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

MapDB::MapDB()
    : Database ( "map.db", "name" )
    , mapNAME  ( _mapNAME )
{
}

///////////////////////////////////////////////////////////////////////////////

MapDB::~MapDB()
{
}

///////////////////////////////////////////////////////////////////////////////

MapRecord&
MapDB::fetchByKey( const string& name, string& err, bool create )
{
    if (name.empty()) {
        err = "is empty";
        return MapRecord::BAD;
    }

    string key = name;
    str::toLower( key );

    const mapNAME_t::iterator found = _mapNAME.find( key );
    if (found != _mapNAME.end())
        return found->second;

    if (!create) {
        err = "not found";
        return MapRecord::BAD;
    }

    // create
    MapRecord& mapRecord = _mapNAME[ key ];
    mapRecord._name = key; // assign correct key

    return mapRecord;
}

///////////////////////////////////////////////////////////////////////////////

void
MapDB::load()
{
    _mapNAME.clear();

    string filename;
    if (open( false, filename ))
        return;

    logBegin( false, filename );

    map<string,string> data;

    // parse each record
    while ( !_stream.rdstate() ) {
        parseData( data );

        map<string,string>::const_iterator it = data.find( _key );
        if ( it == data.end() )
            continue;

        string err;
        MapRecord& mapRecord = fetchByKey( it->second, err, true );
        if ( mapRecord.isNull() ) {
            ostringstream msg;
            msg << "WARNING: skipping invalid map record: " << it->second << endl;
            trap_Printf( msg.str().c_str() );
            _mapNAME.erase( it->second );
            continue;
        }

        mapRecord.decode( data );
    }

    logEnd( _mapNAME.size(), "maps" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

void
MapDB::save()
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
        << '\n' << "## records: " << _mapNAME.size()
        << '\n' << "##"
        << '\n' << "###############################################################################";

    int recnum = 0;

    // Output maps
    const mapNAME_t::iterator max = _mapNAME.end();
    for ( mapNAME_t::iterator it = _mapNAME.begin(); it != max; it++ ) {
        // Write the record
        it->second.encode( _stream, recnum++ );
    }

    _stream << '\n';

    logEnd( _mapNAME.size(), "maps" );
    close();
}

///////////////////////////////////////////////////////////////////////////////

MapDB mapDB;
MapRecord* currentMap;
