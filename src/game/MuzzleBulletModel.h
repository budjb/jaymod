#ifndef GAME_MUZZLEBULLETMODEL_H
#define GAME_MUZZLEBULLETMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * MUZZLE style bullet origins.
 * Bullets fire from muzzle-point of active weapon as offset by MDX animation.
 *
 */
class MuzzleBulletModel : public AbstractBulletModel
{
private:
    MuzzleBulletModel();

protected:
    void adjustStartPoint( vec3_t );

    SingleBulletVolume _bulletVol;

public:
    MuzzleBulletModel( Client&, bool );
    ~MuzzleBulletModel();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_MUZZLEBULLETMODEL_H
