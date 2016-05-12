#ifndef GAME_STANDARDHITMODEL_H
#define GAME_STANDARDHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * STANDARD style hitboxes offer improvement over BASIC.
 * headBox follows animation. Active in all states.
 * torsoBox behaves as Basic.torsoBox. 
 * legBox behaves as Basic.legBox.
 *
 */
class StandardHitModel : public AbstractStateHitModel
{
private:
    StandardHitModel();

    float _torsoAdjust;

protected:
    StandardHitModel* doSnapshot ( );
    void              doState    ( );
    void              doStateRun ( );


public:
    StandardHitModel( Client&, vitality_t );
    ~StandardHitModel();

    StandardHitModel& operator=( const StandardHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_STANDARDHITMODEL_H
