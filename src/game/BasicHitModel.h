#ifndef GAME_BASICHITMODEL_H
#define GAME_BASICHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * BASIC style hitboxes offer a slight improvement over ETMAIN.
 * headBox behaves like Etmain.headBox but is active in all states.
 * torsoBox behaves like Etmain.torsoBox except that it chops height when { DEAD, PLAYDEAD, PRONE }.
 * legBox behaves like Etmain.legBox but is active when when { DEAD, PLAYDEAD, PRONE }.
 *
 */
class BasicHitModel : public AbstractStateHitModel
{
private:
    BasicHitModel();

    float _torsoAdjust;

protected:
    BasicHitModel* doSnapshot ( );
    void           doState    ( );
    void           doStateRun ( );

public:
    BasicHitModel( Client&, vitality_t );
    ~BasicHitModel();

    BasicHitModel& operator=( const BasicHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_BASICHITMODEL_H
