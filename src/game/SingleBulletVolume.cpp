#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

SingleBulletVolume::SingleBulletVolume(
    AbstractBulletModel& bulletModel_,
    bool                 trail_ )

    : AbstractBulletVolume( TYPE_SINGLE, ET_BULLET_VOLUME, bulletModel_, trail_ )
{
}

///////////////////////////////////////////////////////////////////////////////

SingleBulletVolume::~SingleBulletVolume()
{
}

///////////////////////////////////////////////////////////////////////////////

void
SingleBulletVolume::doFire( TraceContext& trx )
{
    _entity->s.modelindex = _colorCode;
    _entity->s.angles2[2] = _alpha;

    // Set scaling of 1x1x1 cube.
    // Use smaller dims for reference model.
    if (_bulletModel.principal)
        VectorSet( _entity->s.origin2, trx.flen, 0.5f, 0.5f );
    else
        VectorSet( _entity->s.origin2, trx.flen, 0.25f, 0.25f );

    // Set origin (which is a CORNER) of 1x1x1 cube.
    VectorCopy( trx.start, _entity->s.origin );

    // Set rotation angles.
    VectorSubtract( trx.fpos, trx.start, _entity->s.angles );
    vectoangles( _entity->s.angles, _entity->s.angles );

    // Must set currentOrigin and re-link otherwise moving out of PVS will prune.
    VectorCopy( _entity->s.origin, _entity->r.currentOrigin );
    trap_LinkEntity( _entity );
}
