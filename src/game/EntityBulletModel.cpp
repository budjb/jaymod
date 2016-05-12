#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

EntityBulletModel::EntityBulletModel( Client& client_, bool reference_ )
    : AbstractBulletModel ( TYPE_ENTITY, client_, reference_ )
    , _bulletVol          ( *this, true )
{
}

///////////////////////////////////////////////////////////////////////////////

EntityBulletModel::~EntityBulletModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
EntityBulletModel::adjustStartPoint( vec3_t start )
{
    VectorCopy( client.gentity.r.currentOrigin, start );
}
