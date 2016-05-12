#ifndef GAME_SINGLEBULLETVOLUME_H
#define GAME_SINGLEBULLETVOLUME_H

///////////////////////////////////////////////////////////////////////////////

class SingleBulletVolume : public AbstractBulletVolume
{
protected:
    void doFire( TraceContext& trx );

public:
    SingleBulletVolume( AbstractBulletModel&, bool );
    ~SingleBulletVolume();
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_SINGLEBULLETVOLUME_H
