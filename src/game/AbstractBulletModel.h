#ifndef GAME_ABSTRACTBULLETMODEL_H
#define GAME_ABSTRACTBULLETMODEL_H

///////////////////////////////////////////////////////////////////////////////

// Macro used to test if MDX should-be active for bullet-model purposes.
#define BULLETMODEL_MDXACTIVE ( \
    ( cvars::bg_bulletmode.ivalue > AbstractBulletModel::_TYPE_MAX_NOMDX ) || \
    ( cvars::g_bulletmodeReference.ivalue > AbstractBulletModel::_TYPE_MAX_NOMDX ))

///////////////////////////////////////////////////////////////////////////////

class Client;

class AbstractBulletModel : public NewAllocator<AbstractBulletModel>
{
private:
    static bool fireWorldAtomic( TraceContext&, bool );

public:
    enum dflags_t {
        DEBUG_BDRAW     = 0x00000001,  // draw bullet-volumes
        DEBUG_RDRAW     = 0x00000002,  // draw reference bullet-model

        DEBUG_LIFECYCLE = 0x00000010,  // log lifecycle (construct/destruct/alloc/free) to client console

        DEBUG_TFIRE     = 0x00000100,  // send fire text to client console
    };

    enum type_t {
        _TYPE_UNDEFINED,

        TYPE_ENTITY,
        TYPE_ETMAIN,

        _TYPE_MAX_NOMDX = TYPE_ETMAIN,

        TYPE_MUZZLE,

        _TYPE_MAX,
    };

    static void cvarTrail( Cvar& );

    static bool factory( AbstractBulletModel*&, Client&, type_t, bool );

    static bool fireWorld( TraceContext& );

    static string  toString( dflags_t );
    static string& toString( dflags_t, string& );

    static string  toString( type_t );
    static string& toString( type_t, string& );

private:
    AbstractBulletModel();

    void updateVisibility();

    bool                 _visible;
    AbstractBulletModel* _reference;

protected:
    AbstractBulletModel( type_t, Client&, bool );

    list<AbstractBulletVolume*> _bulletVolumeList;

public:
    virtual ~AbstractBulletModel();

    virtual void adjustStartPoint     ( vec3_t ) = 0;
    void         firePlayer           ( TraceContext& );
    void         registerBulletVolume ( AbstractBulletVolume& );
    void         run                  ( );

    const type_t type;
    const bool   principal;
    Logger       debug;
    Client&      client;
    const bool&  visible;
};

///////////////////////////////////////////////////////////////////////////////
//
#include <game/EntityBulletModel.h>
#include <game/EtmainBulletModel.h>
#include <game/MuzzleBulletModel.h>

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ABSTRACTBULLETMODEL_H
