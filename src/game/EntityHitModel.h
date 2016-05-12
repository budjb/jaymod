#ifndef GAME_ENTITYHITMODEL_H
#define GAME_ENTITYHITMODEL_H

///////////////////////////////////////////////////////////////////////////////

/*
 * ENTITY style hitboxes offer the simplest kind of hitbox which is simply
 * tracks the actual axis-aligned bounding-box of the player entity.
 * This box is the players collision-box used in most of the game's physics
 * code.
 *
 * This is primarily used as a development aid and not intended for game play.
 * It makes an excellent reference.
 *
 */
class EntityHitModel : public AbstractHitModel
{
private:
    EntityHitModel();

protected:
    void            doRun      ( );
    EntityHitModel* doSnapshot ( );

    AlignedCuboidHV _mainBox;

public:
    EntityHitModel( Client&, vitality_t );
    ~EntityHitModel();

    EntityHitModel& operator=( const EntityHitModel& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ENTITYHITMODEL_H
