#ifndef GAME_ADVANCEDHITMODEL_H
#define GAME_ADVANCEDHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * Advanced style hitboxes.
 * All boxes follow animated player model tags.
 *
 */
class AdvancedHitModel : public AbstractHitModel
{
private:
    AdvancedHitModel();

protected:
    void              doRun      ( );
    AdvancedHitModel* doSnapshot ( );

    AlignedCuboidHV _headBox;
    AlignedCuboidHV _armLeftBox;
    AlignedCuboidHV _armRightBox;
    AlignedCuboidHV _handLeftBox;
    AlignedCuboidHV _handRightBox;
    AlignedCuboidHV _torsoLeftBox;
    AlignedCuboidHV _torsoRightBox;
    AlignedCuboidHV _footLeftBox;
    AlignedCuboidHV _footRightBox;

public:
    AdvancedHitModel( Client&, vitality_t );
    ~AdvancedHitModel();

    AdvancedHitModel& operator=( const AdvancedHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ADVANCEDHITMODEL_H
