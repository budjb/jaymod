#ifndef GAME_ENTITYBULLETMODEL_H
#define GAME_ENTITYBULLETMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * ENTITY style bullet origins.
 * Bullets fire from entity origin.
 *
 * This is primarily used as a development aid and not intended for game play.
 * It makes an excellent reference.
 *
 */
class EntityBulletModel : public AbstractBulletModel
{
private:
    EntityBulletModel();

protected:
    void adjustStartPoint( vec3_t );

    SingleBulletVolume _bulletVol;

public:
    EntityBulletModel( Client&, bool );
    ~EntityBulletModel();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ENTITYBULLETMODEL_H
