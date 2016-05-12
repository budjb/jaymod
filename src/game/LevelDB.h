#ifndef GAME_LEVELDB_H
#define GAME_LEVELDB_H

///////////////////////////////////////////////////////////////////////////////

class LevelDB : public Database {
public:
    typedef map<int,Level> mapLEVEL_t;

private:
    mapLEVEL_t _mapLEVEL; // memory-map for all records

public:
    LevelDB();
    ~LevelDB();

    void load();
    void save();

    Level& fetchByKey  ( int, string&, bool = false );
    Level& fetchByKey  ( const string&, string&, bool = false );
    Level& fetchByName ( const string&, string& );
    uint32 remove      ( Level&, const Level& );

    const mapLEVEL_t& mapLEVEL;

private:
    int toKey( const string& );
};

///////////////////////////////////////////////////////////////////////////////

extern LevelDB levelDB;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_LEVELDB_H
