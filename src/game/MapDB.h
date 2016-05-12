#ifndef GAME_MAPDB_H
#define GAME_MAPDB_H

///////////////////////////////////////////////////////////////////////////////

class MapDB : public Database {
public:
    typedef map<const string,MapRecord> mapNAME_t;

private:
    mapNAME_t _mapNAME;

public:
    MapDB();
    ~MapDB();

    void load ();  // loads all records from disk
    void save ();  // saves all records to disk

    MapRecord& fetchByKey( const string&, string&, bool = false );

    const mapNAME_t& mapNAME;
};

///////////////////////////////////////////////////////////////////////////////

extern MapDB mapDB;
extern MapRecord*  currentMap;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_MAPDB_H
