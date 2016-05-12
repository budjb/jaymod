#ifndef GAME_ABSTRACTSTATEHITMODEL_H
#define GAME_ABSTRACTSTATEHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * Abstract implementation for hitmodels following player state.
 * This is older technology and state changes are instantaneous.
 * Also has the characteristic of having a order-based hitbox test.
 *
 * NOTE: box construction order is important as per our specialization of doTraceBullet().
 *
 */
class AbstractStateHitModel : public AbstractHitModel
{
public:
    typedef enum {
        _STATE_UNDEFINED,

        STATE_CROUCH,
        STATE_DEAD,
        STATE_PLAYDEAD,
        STATE_PRONE,
        STATE_STAND,

        _STATE_MAX,
    } state_t;

    static string  toString( state_t );
    static string& toString( state_t, string& );

private:
    AbstractStateHitModel();

protected:
    AbstractStateHitModel( type_t, Client&, vitality_t );

    void               doRun         ( );
    virtual void       doState       ( ) = 0;
    virtual void       doStateRun    ( ) = 0;
    AbstractHitVolume* doTracePlayer ( TraceContext& );

    state_t _state;

    AlignedCuboidHV _headBox;
    AlignedCuboidHV _footBox;
    AlignedCuboidHV _torsoBox;

public:
    virtual ~AbstractStateHitModel();

    AbstractStateHitModel& operator=( const AbstractStateHitModel& );

    const state_t& state;
};

///////////////////////////////////////////////////////////////////////////////

#include <game/BasicHitModel.h>
#include <game/EtmainHitModel.h>
#include <game/StandardHitModel.h>

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ABSTRACTSTATEHITMODEL_H
