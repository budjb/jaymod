#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

MapEntityList::MapEntityList()
    : lastUpdated ( 0 )
{
    init();
}

///////////////////////////////////////////////////////////////////////////////

MapEntityList::~MapEntityList()
{
    clean();
}

///////////////////////////////////////////////////////////////////////////////

void MapEntityList::clean()
{
    for (int i = 0; i < MAX_MAPENTITIES; i++) {
        MapEntity* mEnt = teamList[i];

        if (mEnt)
            delete mEnt;
    }
}

///////////////////////////////////////////////////////////////////////////////

void MapEntityList::init()
{
    memset(teamList, 0, sizeof(teamList));
}

///////////////////////////////////////////////////////////////////////////////

void MapEntityList::reset()
{
    clean();
    init();
}

///////////////////////////////////////////////////////////////////////////////

MapEntity* MapEntityList::findEnt( int num )
{
    if (num < 0 || num >= MAX_MAPENTITIES)
        return NULL;

    for (int i = 0; i < MAX_MAPENTITIES; i++) {
        MapEntity* mEnt = teamList[i];

        if (!mEnt)
            continue;

        if (mEnt->singleClient >= 0)
            continue;

        if (mEnt->entNum == num)
            return mEnt;
    }
    
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

MapEntity* MapEntityList::findEntSingleClient( int entNum, int clientNum, int& pos ) {
    if (pos < 0 || pos >= MAX_MAPENTITIES)
        return NULL;

    for( ; pos < MAX_MAPENTITIES; pos++ ) {
    MapEntity* mEnt = teamList[pos];

    if( clientNum == -1 ) {
        if( mEnt->singleClient < 0 ) {
            continue;
        }
    } else if( mEnt->singleClient >= 0 && clientNum != mEnt->singleClient ) {
        continue;
    }

    if( entNum == mEnt->entNum )
        return mEnt;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
