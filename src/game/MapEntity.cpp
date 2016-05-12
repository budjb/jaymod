#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

MapEntity::MapEntity()
    : yaw           ( 0 )
    , data          ( 0 )
    , type          ( 0 )
    , startTime     ( 0 )
    , singleClient  ( 0 )
    , entNum        ( 0 )

    , pYaw          ( 0 )
    , pData         ( 0 )
    , pType         ( 0 )
    , pStartTime    ( 0 )
    , pSingleClient ( 0 )
    , pEntNum       ( 0 )
    , remove        ( false )
{
    memset(org, 0, sizeof(org));
    memset(pOrg, 0, sizeof(pOrg));
}

///////////////////////////////////////////////////////////////////////////////

MapEntity::~MapEntity()
{

}

///////////////////////////////////////////////////////////////////////////////

// Check whether there is a delta
bool MapEntity::changed()
{
    if (yaw != pYaw || data != pData || type != pType || entNum != pEntNum)
        return true;

    for (int i = 0; i < 3; i++) {
        if (org[i] != pOrg[i])
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

// Update the old values
void MapEntity::reconcile()
{
    pYaw = yaw;
    pData = data;
    pType = type;
    pStartTime = startTime;
    pSingleClient = singleClient;
    pEntNum = entNum;

    for (int i = 0; i < 3; i++) {
        pOrg[i] = org[i];
    }
}
