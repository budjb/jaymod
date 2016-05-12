#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

EntityHitModel::EntityHitModel( Client& client_, vitality_t vitality_ )
    : AbstractHitModel ( TYPE_ENTITY, client_, vitality_ )
    , _mainBox         ( AbstractHitVolume::ZONE_BODY, *this, AbstractHitVolume::SCOPE_PLAYER )
{
}

///////////////////////////////////////////////////////////////////////////////

EntityHitModel::~EntityHitModel()
{
}

///////////////////////////////////////////////////////////////////////////////

void
EntityHitModel::doRun()
{
    // Update main box.
    VectorCopy( client.gentity.r.currentOrigin, _mainBox.mins );
    VectorAdd( _mainBox.mins, client.gentity.r.mins, _mainBox.mins );

    VectorCopy( client.gentity.r.currentOrigin, _mainBox.maxs );
    VectorAdd( _mainBox.maxs, client.gentity.r.maxs, _mainBox.maxs );
}

///////////////////////////////////////////////////////////////////////////////

EntityHitModel&
EntityHitModel::operator=( const EntityHitModel& obj )
{
    AbstractHitModel::operator=( obj );
    _mainBox = obj._mainBox;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

EntityHitModel*
EntityHitModel::doSnapshot()
{
    EntityHitModel* obj = new EntityHitModel( client, VITALITY_GHOST );
    *obj = *this;
    return obj;
}
