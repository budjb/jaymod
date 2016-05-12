#ifndef GAME_ETMAINHITMODEL_H
#define GAME_ETMAINHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * ETMAIN style hitboxes.
 * headBox factors viewangle. Active when { CROUCH, PRONE, STAND ).
 * torsoBox follows entity bounding-box exactly. Active in all states.
 * legBox factors viewangle. Active when { PRONE }.
 *
 */
class EtmainHitModel : public AbstractStateHitModel
{
private:
    EtmainHitModel();

protected:
    EtmainHitModel* doSnapshot ( );
    void            doState    ( );
    void            doStateRun ( );

public:
    EtmainHitModel( Client&, vitality_t );
    ~EtmainHitModel();

    EtmainHitModel& operator=( const EtmainHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ETMAINHITMODEL_H
