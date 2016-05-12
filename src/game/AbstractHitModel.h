#ifndef GAME_ABSTRACTHITMODEL_H
#define GAME_ABSTRACTHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

// Macro used to test if MDX should-be active for hit-model purposes.
#define HITMODEL_MDXACTIVE ( \
    ( cvars::bg_hitmode.ivalue > AbstractHitModel::_TYPE_MAX_NOMDX ) || \
    ( cvars::g_hitmodeReference.ivalue > AbstractHitModel::_TYPE_MAX_NOMDX ))

///////////////////////////////////////////////////////////////////////////////

class Client;

class AbstractHitModel : public NewAllocator<AbstractHitModel>
{
public:
    enum dflags_t {
        DEBUG_HDRAW     = 0x00000001,  // draw hit-volumes
        DEBUG_RDRAW     = 0x00000002,  // draw reference hit-model
        DEBUG_WDRAW     = 0x00000004,  // draw world-volume

        DEBUG_LIFECYCLE = 0x00000010,  // log lifecycle (construct/destruct/alloc/free) to client console
        DEBUG_SNAPSHOT  = 0x00000020,  // log snapshot use to client console
        DEBUG_STATE     = 0x00000040,  // log state changes for state-driven models to client console

        DEBUG_TRAY      = 0x00000100,  // log ray text to client console
        DEBUG_TVOLUME   = 0x00000200,  // log volume text to client console
    };

    enum type_t {
        _TYPE_UNDEFINED,

        TYPE_ENTITY,
        TYPE_ETMAIN,
        TYPE_BASIC,

        _TYPE_MAX_NOMDX = TYPE_BASIC,

        TYPE_STANDARD,
        TYPE_ADVANCED,
        TYPE_ORIENTED,

        _TYPE_MAX,
    };

    enum vitality_t {
        _VITALITY_UNDEFINED,

        VITALITY_PRINCIPAL,
        VITALITY_REFERENCE,
        VITALITY_GHOST,

        _VITALITY_MAX,
    };

    static void cvarAntilag     ( Cvar& );  // cvar-changed callback
    static void cvarAntilagLerp ( Cvar& );  // cvar-changed callback
    static void cvarFat         ( Cvar& );  // cvar-changed callback
    static void cvarGhosting    ( Cvar& );  // cvar-changed callback
    static void cvarZone        ( Cvar& );  // cvar-changed callback

    static bool factory      ( AbstractHitModel*&, Client&, type_t, vitality_t );
    static void ghostCleanup ( );  // Invoked from GAME_SHUTDOWN.
    static void ghostPrune   ( );  // Invoked from GAME_RUNFRAME.
    static bool traceWorld   ( TraceContext& );

    static string  toString ( dflags_t );
    static string& toString ( dflags_t, string& );
    static string  toString ( type_t );
    static string& toString ( type_t, string& );
    static string  toString ( vitality_t );
    static string& toString ( vitality_t, string& );

private:
    AbstractHitModel();

    void lerp             ( AbstractHitModel&, float );
    void recordHit        ( AbstractHitVolume::zone_t );
    void snapshot         ( );
    void snapshotPrune    ( );
    void tracePlayerBegin ( TraceContext& );
    void tracePlayerEnd   ( TraceContext& );
    void updateVisibility ( );

    AbstractHitVolume* volumeForZone( AbstractHitVolume::zone_t );

    bool              _visible;
    AbstractHitModel* _reference;
    int               _time;      // Server time when run().

    list<AbstractHitModel*> _snapshots;  // History of AHM anti-lag snapshots.

    vec3_t            _originalBounds[2];
    AbstractHitModel* _contextHitModel;

protected:
    AbstractHitModel( type_t, Client&, vitality_t );

    virtual void               doRun         ( ) = 0;
    virtual AbstractHitModel*  doSnapshot    ( ) = 0;
    virtual AbstractHitVolume* doTracePlayer ( TraceContext& );

    list<AbstractHitVolume*> _hitVolumeList;

public:
    virtual ~AbstractHitModel();

    virtual AbstractHitModel& operator=( const AbstractHitModel& );

    void registerHitVolume ( AbstractHitVolume& );
    void run               ( );
    bool tracePlayer       ( TraceContext& );

    const type_t     type;
    const vitality_t vitality;
    Logger           debug;
    Client&          client;
    const bool&      visible;
    const int&       time;
    AlignedCuboidHV  worldVol;
};

///////////////////////////////////////////////////////////////////////////////

#include <game/AbstractStateHitModel.h>
#include <game/AdvancedHitModel.h>
#include <game/EntityHitModel.h>
#include <game/OrientedHitModel.h>

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ABSTRACTHITMODEL_H
