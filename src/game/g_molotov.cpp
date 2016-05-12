#include <bgame/impl.h>

namespace molotov {

///////////////////////////////////////////////////////////////////////////////

gentity_t& spawn( gentity_t&, vec3_t, vec3_t );

void destroy ( gentity_t* );
void run     ( gentity_t* );

///////////////////////////////////////////////////////////////////////////////

class Chunk
{
public:
    typedef list<Chunk*> List;

public:
    Chunk();
    ~Chunk();

    void markAvail();
    void markUsed();

    void compute();
    bool process();

    gentity_t* _inflictor;
    int        _inflictorSpawnTime;

    gentity_t* _attacker;
    int        _attackerConnectTime;

    int    _beginTime;
    int    _endTime;
    vec3_t _origin;
    vec3_t _mins;
    vec3_t _maxs;

private:
    void inflictDamage();

    int            _alarmTime;
    List::iterator _iterator;
};

Chunk::List chunkAvail;
Chunk::List chunkUsed;

///////////////////////////////////////////////////////////////////////////////

void
destroy( gentity_t* ent )
{
    g_entityObjects[ent-g_entities].molotovScreamers.clear();
    G_FreeEntity( ent );
}

///////////////////////////////////////////////////////////////////////////////

void
init()
{
}

///////////////////////////////////////////////////////////////////////////////

gentity_t*
launch( gentity_t& actor, int scalarvel )
{
    vec3_t  velocity;
    vec3_t  offset;
    vec3_t  org;
    trace_t tr;

    // Do nothing if dead.
    if (actor.client->ps.pm_type == PM_DEAD || actor.health <= 0)
        return NULL;

    // Do nothing if playing dead.
    if (actor.client->ps.eFlags & EF_PLAYDEAD)
        return NULL;

    // Do nothing if using MG42.
    if (actor.client->ps.persistant[PERS_HWEAPON_USE] && actor.active)
        return NULL;

    AngleVectors( actor.client->ps.viewangles, velocity, NULL, NULL );
    VectorScale( velocity, 34, offset );
    offset[2] += actor.client->ps.viewheight - 5; // a little lower than view
    VectorScale( velocity, scalarvel, velocity );
    velocity[2] += 25 + rand()%35;

    VectorAdd( actor.s.pos.trDelta, velocity, velocity );
    VectorAdd( actor.client->ps.origin, offset, org );

    trap_Trace( &tr, actor.client->ps.origin, molotov::mins, molotov::maxs, org, actor.s.number, MASK_SOLID );

    VectorCopy( tr.endpos, org );
    gentity_t& molotov = spawn( actor, org, velocity );

    // Increase stats
    actor.client->sess.aWeaponStats[WS_MOLOTOV].atts += 1;

    // Lose uniform if can be seen
    if (actor.client->ps.powerups[PW_OPS_DISGUISED] && G_PlayerCanBeSeenByOthers( &actor ))
        actor.client->ps.powerups[PW_OPS_DISGUISED] = 0;

    return &molotov;
}

///////////////////////////////////////////////////////////////////////////////

void
run( gentity_t* ent )
{
    vec3_t origin;
    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin, qfalse, 0 );

    // remove body from mask if no velocity
    if (!ent->s.pos.trDelta[0] && !ent->s.pos.trDelta[1] && !ent->s.pos.trDelta[2])
        ent->clipmask &= ~CONTENTS_BODY;

    /* Main trace loop - only 1 trace is done unless we destroy some entities and retrace
     * Since 2 or 3 iterations is most likely to ever occur, we'll artifically cap at 10 iterations.
     */
    bool impacted = false;
    vec3_t impactvel;
    list<gentity_t*> relinks; // entities that need to be re-linked after loop
    trace_t tr;

    for (int ti = 0; ti < 10; ti++) {
        impacted = false;

        // trace from old position to new position
        memset( &tr, 0, sizeof(tr) );
        trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, ent->clipmask );
        if (tr.startsolid)
            tr.fraction = 0;

        // move ent to new position
        VectorCopy( tr.endpos, ent->r.currentOrigin );
        trap_LinkEntity( ent );

        // nothing to do if no collision
        if (tr.fraction == 1.0f) {
            ent->nextthink = level.time;
            break;
        }

        // free if impact with sky
        if (tr.surfaceFlags & SURF_NOIMPACT) {
            G_FreeEntity( ent );
            break;
        }

        impacted = true;
        const int evtime = level.time + (int)((level.time - ent->s.pos.trTime)*tr.fraction + 0.5f);
        BG_EvaluateTrajectoryDelta( &ent->s.pos, evtime, impactvel, qfalse, 0 );

        // check if explosive (window, fence, etc)
        gentity_t& target = g_entities[tr.entityNum];

        vec3_t dvel;
        VectorCopy( impactvel, dvel ); // make mutable copy because G_Damage will modify

        if (target.takedamage && target.health > 0) {
            switch (target.s.eType) {
                case ET_PLAYER:
                    G_Damage( &target, ent, &g_entities[ent->r.ownerNum], dvel, tr.endpos, 20, 0, ent->methodOfDeath );
                    G_AddEvent( &target, EV_BONK, 0 );
                    break;

                case ET_EXPLOSIVE:
                    G_Damage( &target, ent, &g_entities[ent->r.ownerNum], dvel, tr.endpos, 10, 0, ent->methodOfDeath );
                    // if we killed target then unlink and redo trace to fly through it
                    if (target.health <= 0) {
                        if (target.r.linked) {
                            relinks.push_back( &target );
                            trap_UnlinkEntity( &target );
                        }
                        continue;
                    }
                    break;

                default:
                    break;
            }
        }

        break;
    }

    // process relink list
    const list<gentity_t*>::iterator max = relinks.end();
    for ( list<gentity_t*>::iterator it = relinks.begin(); it != max; it++ )
        trap_LinkEntity( *it );

    if (!impacted)
        return;

    // impacted
    ent->s.pos.trType = TR_STATIONARY;
    VectorCopy( tr.endpos, ent->s.pos.trBase );
    VectorCopy( impactvel, ent->s.pos.trDelta );
    SnapVector( ent->s.pos.trBase );
    SnapVector( ent->s.pos.trDelta );

    // update server-end with final position
    ent->r.contents = 0;
    VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
    trap_LinkEntity( ent );

    // init fireball effect
    ent->s.effect1Time = level.time;  // begin time for fireball
    ent->s.effect2Time = rand();      // sync'd random seed

    // encode surface plane normal of impact (30 bits)
    ent->s.effect3Time  = ((int)(tr.plane.normal[0] * 512.0f) + 511);
    ent->s.effect3Time |= ((int)(tr.plane.normal[1] * 512.0f) + 511) << 10;
    ent->s.effect3Time |= ((int)(tr.plane.normal[2] * 512.0f) + 511) << 20;

    // clear screaming clients (overloaded field)
    ent->s.dmgFlags = 0;

    // install cleanup
    ent->nextthink = level.time + chunkDuration_i + 1000;
    ent->think = destroy;

    // compute reflection vector
    vec3_t delta;
    VectorCopy( ent->s.pos.trDelta, delta );
    VectorNormalize( delta );

    const float rscale = -2.0f * DotProduct( delta, tr.plane.normal );
    vec3_t rdelta;
    VectorMA( delta, rscale, tr.plane.normal, rdelta );

    // compute right/up vectors
    vec3_t right, up;
    PerpendicularVector( right, rdelta );
    CrossProduct( rdelta, right, up );

    // construct chunks
    int seed = ent->s.effect2Time; // sync'd random seed

    vec3_t point;
    VectorCopy( ent->s.pos.trBase, point );

    for (int i = 0; i < chunkMax; i++) {
        if (i) {
#if 1 // damage will happen thru walls
            VectorMA( point, chunkHop, rdelta, point );
            VectorMA( point, chunkHop, delta, point );
#else // prevent damage thru walls (not working)
            vec3_t ptmp;
            VectorMA( point, chunkHop, rdelta, ptmp );
            VectorMA( ptmp, chunkHop, delta, ptmp );

            trace_t tr;
            memset( &tr, 0, sizeof(tr) );
            trap_Trace( &tr, point, ptmp, molotov::mins, molotov::maxs, ent-g_entities, CONTENTS_SOLID );

            if (tr.startsolid)
                break;

            if (tr.fraction != 1.0f)
                break;

            VectorCopy( ptmp, point );
#endif
        }

        if (chunkAvail.empty())
            new Chunk(); // safe: object adds self to list

        Chunk& chunk = *chunkAvail.back();
        chunk.markUsed();

        chunk._inflictor          = ent;
        chunk._inflictorSpawnTime = ent->spawnTime;

        chunk._attacker            = &g_entities[ent->r.ownerNum];
        chunk._attackerConnectTime = chunk._attacker->client ? chunk._attacker->client->pers.connectTime : 0;

        chunk._beginTime = level.time;
        chunk._endTime   = level.time + chunkDuration_i;

        VectorCopy( point, chunk._origin );
        VectorMA( chunk._origin, Q_crandom( &seed ) * chunkSpread, right, chunk._origin );
        VectorMA( chunk._origin, Q_crandom( &seed ) * chunkSpread, up, chunk._origin );

        chunk.compute();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
runChunks()
{
    if (chunkUsed.empty())
        return;

    for ( Chunk::List::iterator it = chunkUsed.begin(); it != chunkUsed.end(); ) {
        Chunk& chunk = **it++;
        if (chunk.process())
            chunk.markAvail();
    }
}

///////////////////////////////////////////////////////////////////////////////

void
shutdown()
{
    // release memory from free chunks
    {
        const Chunk::List::iterator max = chunkAvail.end();
        for ( Chunk::List::iterator it = chunkAvail.begin(); it != max; it++ )
            delete *it;
        chunkAvail.clear();
    }

    // release memory from used chunks
    {
        const Chunk::List::iterator max = chunkUsed.end();
        for ( Chunk::List::iterator it = chunkUsed.begin(); it != max; it++ )
            delete *it;
        chunkUsed.clear();
    }
}

///////////////////////////////////////////////////////////////////////////////

gentity_t&
spawn( gentity_t& actor, vec3_t start, vec3_t dir )
{
    gentity_t& molotov = *G_Spawn();

    molotov.s.eType   = ET_MISSILE;
    molotov.s.teamNum = actor.client->sess.sessionTeam;
    molotov.s.weapon  = WP_MOLOTOV;

    molotov.r.contents = CONTENTS_MISSILECLIP;
    molotov.r.svFlags  = SVF_BROADCAST;
    molotov.r.ownerNum = actor.s.number;

    molotov.classname     = "molotov";
    molotov.clipmask      = MASK_MISSILESHOT;
    molotov.health        = 10;
    molotov.methodOfDeath = MOD_MOLOTOV;
    molotov.parent        = &actor;

    // install run handler
    molotov.nextthink = level.time;
    molotov.think = run;

    // set bounds so item doesn't spawn outside of world
    VectorCopy( molotov::mins, molotov.r.mins );
    VectorCopy( molotov::maxs, molotov.r.maxs );

    // set trajectory
    molotov.s.pos.trType = TR_GRAVITY;
    molotov.s.pos.trTime = level.time;

    VectorCopy( start, molotov.s.pos.trBase );
    VectorCopy( dir, molotov.s.pos.trDelta );

    // add velocity of ground entity
    if (actor.s.groundEntityNum != ENTITYNUM_NONE && actor.s.groundEntityNum != ENTITYNUM_WORLD )
        VectorAdd( molotov.s.pos.trDelta, g_entities[actor.s.groundEntityNum].instantVelocity, molotov.s.pos.trDelta );

    SnapVector( molotov.s.pos.trBase );
    SnapVector( molotov.s.pos.trDelta );

    VectorCopy( molotov.s.pos.trBase, molotov.r.currentOrigin );

    return molotov;
}

///////////////////////////////////////////////////////////////////////////////

Chunk::Chunk()
    : _endTime( 0 )
{
    memset( _origin, 0, sizeof(_origin) );

    chunkAvail.push_front( this );
    _iterator = chunkAvail.begin();
}

///////////////////////////////////////////////////////////////////////////////

Chunk::~Chunk()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Chunk::compute()
{
    _alarmTime = 0;

    const float boxradius = chunkRadius * 1.41421356f;

    _mins[0] = _origin[0] - boxradius;
    _mins[1] = _origin[1] - boxradius;
    _mins[2] = _origin[2] - boxradius;

    _maxs[0] = _origin[0] + boxradius;
    _maxs[1] = _origin[1] + boxradius;
    _maxs[2] = _origin[2] + boxradius;
}

///////////////////////////////////////////////////////////////////////////////

void
Chunk::inflictDamage()
{
    static const int maxEnts = MAX_CLIENTS * 2;

    set<int>& screamers = g_entityObjects[_inflictor-g_entities].molotovScreamers;

    // expire screamers
    {
        // build expired list
        list<int> expired;
        {
            const set<int>::iterator max = screamers.end();
            for ( set<int>::iterator it = screamers.begin(); it != max; it++ ) {
                gentity_t& ent = g_entities[*it];
                if (level.time < ent.nextMolotovScreamTime)
                    continue;
                expired.push_back( *it );
            }
        }

        // expire
        {
            const list<int>::iterator max = expired.end();
            for ( list<int>::iterator it = expired.begin(); it != max; it++ )
                screamers.erase( *it );
        }
    }

    int ents[maxEnts];
    const int num = trap_EntitiesInBox( _mins, _maxs, ents, maxEnts );
    for (int i = 0; i < num; i++ ) {
        const int entnum = ents[i];
        gentity_t& ent = g_entities[entnum];

        // skip if cannot take damage
        if (!ent.takedamage && (!ent.dmgparent || !ent.dmgparent->takedamage))
            continue;

        // skip if underwater
        if (ent.waterlevel == 3)
            continue;

        vec3_t v;
        G_AdjustedDamageVec( &ent, _origin, v );

        float dist = VectorLength( v );
        if (dist >= chunkRadius)
            continue;

        const int points = (int)(40.0f * (1.0f - dist / chunkRadius ));

        if (!CanDamage( &ent, _origin ))
            continue;

        G_Damage( &ent, _inflictor, _attacker, v, _origin, points, DAMAGE_RADIUS, MOD_MOLOTOV );

        if (ent.client
            && ent.health > 0
            && (&ent == _attacker || (g_friendlyFire.integer && !OnSameTeam( &ent, _inflictor )))
            && (level.time >= ent.nextMolotovScreamTime)
            && screamers.size() < 4)
        {
            ent.nextMolotovScreamTime = level.time + molotov::screamDuration;
            screamers.insert( entnum );
        }
    }

    // encode screamers
    int& field = _inflictor->s.dmgFlags;
    field = 0;

    int count = 0;
    const set<int>::iterator max = screamers.end();
    for ( set<int>::iterator it = screamers.begin(); it != max; it++, count++ ) {
        if (count >= 4)
            break;

        const int slot = *it;
        field |= (0x40 | (slot & 0x3f)) << (7*count); // hi-order bit indicates if slot is crying
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Chunk::markAvail()
{
    chunkUsed.erase( _iterator );
    chunkAvail.push_front( this );
    _iterator = chunkAvail.begin();
}

///////////////////////////////////////////////////////////////////////////////

void
Chunk::markUsed()
{
    chunkAvail.erase( _iterator );
    chunkUsed.push_front( this );
    _iterator = chunkUsed.begin();
}

///////////////////////////////////////////////////////////////////////////////

bool
Chunk::process()
{
    // bail if too early
    if (level.time < _alarmTime)
        return false;

    _alarmTime = level.time + 250;

    // bail if expired and indicate we desire to be back on free list
    if (!(level.time < _endTime))
        return true;

    // bail if molotov entity has died or respawned
    if (_inflictor->spawnTime != _inflictorSpawnTime)
        return true;

    if (_attacker->client) {
        // if attacker is has since disconnected
        if (_attacker->client->pers.connected != CON_CONNECTED)
            return true;

        // if attacker has reconnected
        if (_attacker->client->pers.connectTime != _attackerConnectTime)
            return true;
    }

    inflictDamage();
    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace molotov
