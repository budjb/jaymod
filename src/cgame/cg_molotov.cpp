#include <bgame/impl.h> 

extern qhandle_t flameShaders[];

namespace molotov {

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

    bool process();

    int    _beginTime;
    int    _endTime;
    vec3_t _origin;

private:
    List::iterator _iterator;
};

Chunk::List chunkAvail;
Chunk::List chunkUsed;

///////////////////////////////////////////////////////////////////////////////

void
draw( centity_t& cent )
{
    const weaponInfo_t& info = cg_weapons[WP_MOLOTOV];

    if (!cent.currentState.effect1Time) {
        // add trail
        if (info.missileTrailFunc)
            info.missileTrailFunc( &cent, &info );

        // add dynamic light
        if (info.missileDlight) {
            trap_R_AddLightToScene( cent.lerpOrigin, info.missileDlight, 1.0f,
                info.missileDlightColor[0], info.missileDlightColor[1], info.missileDlightColor[2],
                cgs.media.whiteShader, 0 );
        }

        // populate render entity
        refEntity_t& re = cent.refEnt;
        memset( &re, 0, sizeof(refEntity_t) );
        AxisClear( re.axis );

        VectorCopy( cent.lerpOrigin, re.origin );
        VectorCopy( cent.lerpOrigin, re.oldorigin );

        re.hModel = info.missileModel;

        // rotate pitch (relative to trajectory)
        vec3_t delta;
        VectorCopy( cent.currentState.pos.trDelta, delta );
        const float spinf = (VectorNormalize( delta ) - 300.0f) / 600.0f;

        vec3_t angles;
        vectoangles( delta, angles );

        angles[PITCH] += (cg.time - cent.currentState.effect1Time) * spinf;
        AnglesToAxis( angles, re.axis );

        // add to scene
        trap_R_AddRefEntityToScene( &re );

        return;
    }

    // add breaking sound
    if (!cent.miscTime) {
        cent.miscTime = cent.currentState.effect1Time;
        trap_S_StartSoundVControl( NULL, cent.currentState.number, CHAN_AUTO, info.overheatSound, 255 );

        // decode surface plane normal of impact (30 bits)
        vec3_t nplane = {
            (( cent.currentState.effect3Time        & 0x3ff) - 511) / 512.0f, 
            (((cent.currentState.effect3Time >> 10) & 0x3ff) - 511) / 512.0f,
            (((cent.currentState.effect3Time >> 20) & 0x3ff) - 511) / 512.0f };

        // compute reflection vector
        vec3_t delta;
        VectorCopy( cent.currentState.pos.trDelta, delta );
        VectorNormalize( delta );

        const float rscale = -2.0f * DotProduct( delta, nplane );
        vec3_t rdelta;
        VectorMA( delta, rscale, nplane, rdelta );

        // compute right/up vectors
        vec3_t right, up;
        PerpendicularVector( right, rdelta );
        CrossProduct( rdelta, right, up );

        // construct chunks
        int seed = cent.currentState.effect2Time; // sync'd random seed
        vec3_t point;
        VectorCopy( cent.lerpOrigin, point );

        for (int i = 0; i < chunkMax; i++) {
            if (i) {
#if 1
                VectorMA( point, chunkHop, rdelta, point );
                VectorMA( point, chunkHop, delta, point );
#else

                vec3_t ptmp;
                VectorMA( point, chunkHop, rdelta, ptmp );
                VectorMA( ptmp, chunkHop, delta, ptmp );

                trace_t tr;
                memset( &tr, 0, sizeof(tr) );
                trap_CM_BoxTrace( &tr, point, ptmp, molotov::mins, molotov::maxs, 0, CONTENTS_SOLID );

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

            chunk._beginTime = cent.miscTime;
            chunk._endTime   = cent.miscTime + chunkDuration_i;

            VectorCopy( point, chunk._origin );
            VectorMA( chunk._origin, Q_crandom( &seed ) * chunkSpread, right, chunk._origin );
            VectorMA( chunk._origin, Q_crandom( &seed ) * chunkSpread, up, chunk._origin );
        }
    }

    const int chunkTime = cg.time - cent.miscTime;
    if (chunkTime >= chunkDuration_i)
        return;

    // loop fire sound - volume falls off after half way point
    const float progress = chunkTime / chunkDuration_f;

    int volume = 255;
    if (progress > 0.5f && progress <= 1.0f)
        volume -= (int)((progress - 0.5f) * 2.0f * 255);

    trap_S_AddLoopingSound( cent.lerpOrigin, vec3_origin, cgs.media.flameStreamSound, volume, 0 );

    // decode screamers and see if any new ones have shown up
    for (int i = 0; i < 4; i++) {
        const int tmp = cent.currentState.dmgFlags >> (7*i);
        if (!(tmp & 0x40))
            continue;

        // skip if victim recently screamed from a molotov
        centity_t& victim = cg_entities[tmp & 0x3f];
        if (cg.time < victim.nextMolotovScreamTime)
            continue;

        victim.nextMolotovScreamTime = cg.time + (screamDuration+500);

        trap_S_StartSoundVControl( NULL, victim.currentState.number, CHAN_AUTO,
            cgs.media.molotovScream[ rand() % 5 ], 255 );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
populatePolyBuffers()
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
restart()
{
    shutdown();
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
    // if expired bail and indicate we desire to be back on free list
    if (!(cg.time < _endTime))
        return true;

    const int elapsed = cg.time - _beginTime;
    const int idx = (int)(((elapsed % chunkDuration_i) / chunkDuration_f) * chunkNumFrames);
    polyBuffer_t* ptmp = CG_PB_FindFreePolyBuffer( flameShaders[idx], 4, 6 );
    if (!ptmp)
        return false;

    polyBuffer_t& pbuf = *ptmp;

    vec4_t* xyz = pbuf.xyz + pbuf.numVerts;
    VectorMA( _origin, -chunkRadius,   cg.refdef_current->viewaxis[1], xyz[0] ); // left
    VectorMA( xyz[0],  -chunkRadius,   cg.refdef_current->viewaxis[2], xyz[0] ); // down
    VectorMA( xyz[0],  chunkRadius_2,  cg.refdef_current->viewaxis[2], xyz[1] ); // up
    VectorMA( xyz[1],  chunkRadius_2,  cg.refdef_current->viewaxis[1], xyz[2] ); // right
    VectorMA( xyz[2],  -chunkRadius_2, cg.refdef_current->viewaxis[2], xyz[3] ); // down

    vec2_t* st = pbuf.st + pbuf.numVerts;
    Vector2Set( st[0], 0.0f, 0.0f );
    Vector2Set( st[1], 0.0f, 1.0f );
    Vector2Set( st[2], 1.0f, 1.0f );
    Vector2Set( st[3], 1.0f, 0.0f );

    byte(*color)[4] = pbuf.color + pbuf.numVerts;
    Vector4Set( color[0], 0, 0, 0, 128 );
    Vector4Set( color[1], 0, 0, 0, 128 );
    Vector4Set( color[2], 0, 0, 0, 128 );
    Vector4Set( color[3], 0, 0, 0, 128 );

    int* index = pbuf.indicies + pbuf.numIndicies;
    index[0] = pbuf.numVerts + 0; // triangle-0
    index[1] = pbuf.numVerts + 1;
    index[2] = pbuf.numVerts + 2;

    index[3] = pbuf.numVerts + 2; // triangle-1
    index[4] = pbuf.numVerts + 3;
    index[5] = pbuf.numVerts + 0;

    pbuf.numVerts    += 4;
    pbuf.numIndicies += 6;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace molotov
