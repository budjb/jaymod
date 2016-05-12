#ifndef GAME_ETMAINBULLETMODEL_H
#define GAME_ETMAINBULLETMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * ETMAIN style bullet origins.
 * Bullets fire from player's viewpoint.
 *
 */
class EtmainBulletModel : public AbstractBulletModel
{
private:
    EtmainBulletModel();

protected:
    void adjustStartPoint( vec3_t );

    SingleBulletVolume _bulletVol;

public:
    EtmainBulletModel( Client&, bool );
    ~EtmainBulletModel();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ETMAINBULLETMODEL_H
