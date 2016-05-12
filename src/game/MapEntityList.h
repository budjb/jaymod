#ifndef GAME_MAPENTITYLIST_H
#define GAME_MAPENTITYLIST_H

///////////////////////////////////////////////////////////////////////////////

#define MAX_MAPENTITIES 1024

class MapEntityList {

public:
    MapEntityList  ( );
    ~MapEntityList ( );

    MapEntity* findEnt             ( int );
    MapEntity* findEntSingleClient ( int, int, int& );

    void reset( );

protected:
    MapEntity* teamList[MAX_MAPENTITIES];

    int lastUpdated;

private:
    void clean ( );
    void init  ( );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_MAPENTITYLIST_H
