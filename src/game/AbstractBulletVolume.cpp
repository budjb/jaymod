#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

AbstractBulletVolume::AbstractBulletVolume(
    type_t               type_,
    entityType_t         entityType_,
    AbstractBulletModel& bulletModel_,
    bool                 trail_ )

    : _trail       ( trail_ )
    , _bulletModel ( bulletModel_ )
    , _entity      ( 0 )
    , _colorCode   ( 0 )
    , _alpha       ( 0.75f )
    , type         ( type_ )
    , entityType   ( entityType_ )
    , flags        ( BVF_NONE )
{
    if (_trail)
        _bulletModel.registerBulletVolume( *this );
}

///////////////////////////////////////////////////////////////////////////////

AbstractBulletVolume::~AbstractBulletVolume()
{
    entityFree();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletVolume::entityAlloc()
{
    if (_entity)
        return;

    _entity = G_Spawn();

    _entity->s.eType       = entityType;
    _entity->s.clientNum   = _bulletModel.client.slot;
    _entity->s.modelindex  = _bulletModel.principal ? 0 : 3;
    _entity->s.effect1Time = 0;
    _entity->parent        = &_bulletModel.client.gentity;

    if (cvars::g_bulletmodeDebug.ivalue & AbstractBulletModel::DEBUG_LIFECYCLE)
        _bulletModel.debug << "entityAlloc: " << _entity->s.number << endl;

    const deque<AbstractBulletVolume*>::iterator end = _trailList.end();
    for ( deque<AbstractBulletVolume*>::iterator it = _trailList.begin(); it != end; it++ )
        (*it)->entityAlloc();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletVolume::entityCompute()
{
    if (!_entity)
        return;

    _entity->s.groundEntityNum = flags;

    if (!_trail)
        return;

    const int delta = cvars::g_bulletmodeTrail.ivalue - int(_trailList.size());

    if (delta > 0) {
        // Trail list needs to grow.
        for (int i = 0; i < delta; i++) {
            AbstractBulletVolume* bv;
            factory( bv, type, _bulletModel, false );
            bv->entityAlloc();
            _trailList.push_back( bv );
        }
    }
    else if (delta < 0) {
        // Trail list needs to shrink.
        for (int i = 0; i > delta; i--) {
            AbstractBulletVolume& bv = *_trailList.back();
            _trailList.pop_back();
            delete &bv;
        }
    }

    const deque<AbstractBulletVolume*>::iterator end = _trailList.end();
    for ( deque<AbstractBulletVolume*>::iterator it = _trailList.begin(); it != end; it++ )
        (*it)->entityCompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletVolume::entityFree()
{
    if (!_entity)
        return;

    const deque<AbstractBulletVolume*>::reverse_iterator end = _trailList.rend();
    for ( deque<AbstractBulletVolume*>::reverse_iterator it = _trailList.rbegin(); it != end; it++ )
        (*it)->entityFree();

    if (cvars::g_bulletmodeDebug.ivalue & AbstractBulletModel::DEBUG_LIFECYCLE)
        _bulletModel.debug << "entityFree: " << _entity->s.number << endl;

    G_FreeEntity( _entity );
    _entity = 0;
}

///////////////////////////////////////////////////////////////////////////////

bool
AbstractBulletVolume::factory(
    AbstractBulletVolume*& result,
    type_t                 type,
    AbstractBulletModel&   bulletModel,
    bool                   trail )
{
    switch (type) {
        case TYPE_SINGLE:
            result = new SingleBulletVolume( bulletModel, trail );
            break;

        default:
            result = new SingleBulletVolume( bulletModel, trail );
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
AbstractBulletVolume::fire( TraceContext& trx )
{
    if (!_entity)
        return;

    flags |= BVF_ENABLED;

    // Update trail.
    if (_trailList.size()) {
        AbstractBulletVolume* bv = _trailList.back();
        _trailList.pop_back();
        _trailList.push_front( bv );

        // Mike TODO: this should be a vritual function call to update info.
        // Or maybe re-work to invoke each bv.fire() .

        bv->_entity->s.modelindex = _colorCode;
        bv->flags |= BVF_ENABLED;

        if (bv->_entity) {
            VectorCopy( _entity->s.origin,  bv->_entity->s.origin );
            VectorCopy( _entity->s.origin2, bv->_entity->s.origin2 );
            VectorCopy( _entity->s.angles,  bv->_entity->s.angles );

            // Must set currentOrigin and re-link otherwise moving out of PVS will prune.
            VectorCopy( _entity->s.origin, bv->_entity->r.currentOrigin );
            trap_LinkEntity( bv->_entity );
        }

        // Apply fading to trail.
        const float adec = 0.25f / (_trailList.size() + 1);
        float aval = 0.5f;
        const deque<AbstractBulletVolume*>::iterator end = _trailList.end();
        for ( deque<AbstractBulletVolume*>::iterator it = _trailList.begin(); it != end; it++ ) {
            AbstractBulletVolume& bv = **it;
            bv._entity->s.angles2[2] = aval;
            aval -= adec;
        }
    }

    _colorCode = _bulletModel.principal
        ? (trx.resultIsPlayer() ? 2 : 1)
        : (trx.resultIsPlayer() ? 4 : 3);

    doFire( trx );
}

///////////////////////////////////////////////////////////////////////////////

string
AbstractBulletVolume::toString( type_t value )
{
    string result;
    return toString( value, result );
}

///////////////////////////////////////////////////////////////////////////////

string&
AbstractBulletVolume::toString( type_t value, string& out )
{
    switch (value) {
        case TYPE_SINGLE:   out = "SINGLE";   return out;
        case TYPE_BUCKSHOT: out = "BUCKSHOT"; return out;

        default:
            break;
    }

    ostringstream oss;
    oss << "UNDEFINED(" << value << ")";
    out = oss.str();
    return out;
}
