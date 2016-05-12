#ifndef GAME_ABSTRACTBULLETVOLUME_H
#define GAME_ABSTRACTBULLETVOLUME_H

///////////////////////////////////////////////////////////////////////////////

class AbstractBulletModel;

///////////////////////////////////////////////////////////////////////////////

/*
 * BulletVolume is an abstract class which represents a bullet ray.
 *
 * To play nice with the game engine, all BVs are finite with { start, end }
 * points. External code such as BulletModel is responsible for updating
 * these fields directly.
 *
 */
class AbstractBulletVolume
{
public:
    enum type_t {
        _TYPE_UNDEFINED,

        TYPE_SINGLE,
        TYPE_BUCKSHOT,

        _TYPE_MAX,
    };

    static bool factory( AbstractBulletVolume*&, type_t, AbstractBulletModel&, bool );

    static string  toString( type_t );
    static string& toString( type_t, string& );

private:
    const bool                   _trail;
    deque<AbstractBulletVolume*> _trailList;

protected:
    AbstractBulletVolume( type_t, entityType_t, AbstractBulletModel&, bool );

    AbstractBulletModel& _bulletModel;
    gentity_t*           _entity;
    int                  _colorCode;
    float                _alpha;

    virtual void doFire( TraceContext& ) = 0;

public:
    virtual ~AbstractBulletVolume();

    void entityAlloc   ( );
    void entityCompute ( );
    void entityFree    ( );
    void fire          ( TraceContext& );

    const type_t       type;
    const entityType_t entityType;
    int                flags;

    vec3_t start;  // absolute world space
    vec3_t end;    // absolute world space
};

///////////////////////////////////////////////////////////////////////////////

#include <game/SingleBulletVolume.h>

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ABSTRACTBULLETVOLUME_H
