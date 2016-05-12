#ifndef GAME_ORIENTEDHITMODEL_H
#define GAME_ORIENTEDHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * Oriented style hitboxes.
 * All boxes follow animated player model tags.
 *
 */
class OrientedHitModel : public AbstractHitModel
{
private:
    OrientedHitModel();

protected:
    void              doRun      ( );
    OrientedHitModel* doSnapshot ( );

    OrientedCuboidHV _headBox;
    OrientedCuboidHV _armLeftBox;
    OrientedCuboidHV _armRightBox;
    OrientedCuboidHV _handLeftBox;
    OrientedCuboidHV _handRightBox;
    OrientedCuboidHV _torsoBox;
    OrientedCuboidHV _legLeftBox;
    OrientedCuboidHV _legRightBox;
    OrientedCuboidHV _footLeftBox;
    OrientedCuboidHV _footRightBox;

public:
    OrientedHitModel( Client&, vitality_t );
    ~OrientedHitModel();

    OrientedHitModel& operator=( const OrientedHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ORIENTEDHITMODEL_H
