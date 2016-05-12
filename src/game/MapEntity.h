#ifndef GAME_MAPENTITY_H
#define GAME_MAPENTITY_H

///////////////////////////////////////////////////////////////////////////////

class MapEntity {

public:
    MapEntity  ( );
    ~MapEntity ( );

    bool changed   ( );
    void reconcile ( );

    // New values
    vec3_t org;
    int    yaw;
    int    data;
    int    type;
    int    startTime;
    int    singleClient;
    int    entNum;

private:
    // Stored values
    vec3_t pOrg;
    int    pYaw;
    int    pData;
    int    pType;
    int    pStartTime;
    int    pSingleClient;
    int    pEntNum;

    // Remove this entity after transmission
    bool remove;
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_MAPENTITY_H
