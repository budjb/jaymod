#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

AlignedCuboidHV::AlignedCuboidHV( zone_t zone_, AbstractHitModel& hitModel_, scope_t scope_ )
    : AbstractHitVolume( TYPE_ALIGNED_CUBOID, zone_, ET_ALIGNED_HIT_VOLUME, hitModel_, scope_ )
{
}

///////////////////////////////////////////////////////////////////////////////

AlignedCuboidHV::~AlignedCuboidHV()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
AlignedCuboidHV::castRay( TraceContext& trx, vec3_t& rpos, float& rlen )
{
    vec3_t quad[4];

    vec3_t tmppos;
    float  tmplen;

    // SOUTH-FACE
    VectorSet( quad[0], mins[0], mins[1], mins[2] );
    VectorSet( quad[1], mins[0], mins[1], maxs[2] );
    VectorSet( quad[2], maxs[0], mins[1], maxs[2] );
    VectorSet( quad[3], maxs[0], mins[1], mins[2] );
    if (castRayPlane( trx, tmppos, tmplen, "SOUTH", quad, 0, 2 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // NORTH-FACE
    VectorSet( quad[0], mins[0], maxs[1], mins[2] );
    VectorSet( quad[1], maxs[0], maxs[1], mins[2] );
    VectorSet( quad[2], maxs[0], maxs[1], maxs[2] );
    VectorSet( quad[3], mins[0], maxs[1], maxs[2] );
    if (castRayPlane( trx, tmppos, tmplen, "NORTH", quad, 0, 2 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // WEST-FACE
    VectorSet( quad[0], mins[0], mins[1], mins[2] );
    VectorSet( quad[1], mins[0], maxs[1], mins[2] );
    VectorSet( quad[2], mins[0], maxs[1], maxs[2] );
    VectorSet( quad[3], mins[0], mins[1], maxs[2] );
    if (castRayPlane( trx, tmppos, tmplen, "WEST", quad, 1, 2 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // EAST-FACE
    VectorSet( quad[0], maxs[0], mins[1], mins[2] );
    VectorSet( quad[1], maxs[0], mins[1], maxs[2] );
    VectorSet( quad[2], maxs[0], maxs[1], maxs[2] );
    VectorSet( quad[3], maxs[0], maxs[1], mins[2] );
    if (castRayPlane( trx, tmppos, tmplen, "EAST", quad, 1, 2 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // TOP-FACE
    VectorSet( quad[0], mins[0], mins[1], maxs[2] );
    VectorSet( quad[1], mins[0], maxs[1], maxs[2] );
    VectorSet( quad[2], maxs[0], maxs[1], maxs[2] );
    VectorSet( quad[3], maxs[0], mins[1], maxs[2] );
    if (castRayPlane( trx, tmppos, tmplen, "TOP", quad, 0, 1 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // BOTTOM-FACE
    VectorSet( quad[0], mins[0], mins[1], mins[2] );
    VectorSet( quad[1], maxs[0], mins[1], mins[2] );
    VectorSet( quad[2], maxs[0], maxs[1], mins[2] );
    VectorSet( quad[3], mins[0], maxs[1], mins[2] );
    if (castRayPlane( trx, tmppos, tmplen, "BOTTOM", quad, 0, 1 )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
AlignedCuboidHV::castRayPlane(
    TraceContext&  trx,
    vec3_t         rpos,
    float&         rlen,
    const string&  name,
    vec3_t*        quad,
    int            iboundx,
    int            iboundy )
{
    vec3_t& qmins = quad[0];
    vec3_t& qmaxs = quad[2];

    // Compute primary trace ray.
    vec3_t ptray;
    VectorSubtract( trx.end, trx.start, ptray );

    // Compute face-normal.
    vec3_t corner[2];
    VectorSubtract( quad[0], quad[1], corner[0] ); 
    VectorSubtract( quad[0], quad[3], corner[1] );

    vec3_t facen;
    CrossProduct( corner[0], corner[1], facen );
    VectorNormalize( facen );

    // Compute plane distance from origin.
    float dist = -DotProduct( facen, quad[0] );

    // Compute (optional) pruning value.
    float rstart = DotProduct( facen, trx.start ) + dist;

    const bool doDebug =
        (cvars::g_hitmodeDebug.ivalue & AbstractHitModel::DEBUG_TVOLUME) &&
        (cvars::g_hitmodeZone.ivalue == zone);

    using namespace text;

    InlineText colA;
    if (doDebug) {
        colA.flags |= ios::left;
        colA.width = 9;
        colA.suffixOutside = " = ";

        trx.debug << xheader( JAYMOD_FUNCTION )
            << "\n(" << xcpush << xcheader << " zone=" << xvalueBOLD( toString( zone ))
                                           << " type=" << xvalueBOLD( toString( type ))
                                           << " face=" << xvalueBOLD( name )
                                           << xcpop << " )"
            << xlindent
            << "\n" << colA( "quad[0]"   ) << xvec3( quad[0] )
            << "\n" << colA( "quad[1]"   ) << xvec3( quad[1] )
            << "\n" << colA( "quad[2]"   ) << xvec3( quad[2] )
            << "\n" << colA( "quad[3]"   ) << xvec3( quad[3] )
            << "\n" << colA( "ptray"     ) << xvec3( ptray )
            << "\n" << colA( "corner[0]" ) << xvec3( corner[0] )
            << "\n" << colA( "corner[1]" ) << xvec3( corner[1] )
            << "\n" << colA( "facen"     ) << xvec3( facen )
            << "\n" << colA( "dist"      ) << xvalue( dist )
            << "\n" << colA( "rstart"    ) << xvalue( rstart );
    }

/* This has been disabled since there are reports of missed shots
 * during close-proximity duels.
 */
#if 0
    // Prune if start point is on backside of face.
    if (rstart > 0) {
        if (doDebug) {
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "back-side of plane" )
                      << xlunindent << "\n";
        }
        return false;
    }
#endif

    // Compute (optional) pruning value.
    float rend = DotProduct( facen, trx.end   ) + dist;

    if (doDebug)
        trx.debug << "\n" << colA( "rend" ) << xvalue( rend );

    // Prune if both points are on same side of plane.
    if ( (rstart < 0 && rend < 0) || (rstart > 0 && rend > 0) ) {
        if (doDebug) {
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "never crosses plane" )
                      << xlunindent << "\n";
        }
        return false;
    }

    // Compute intersection point of infinite line on infinite plane.
    float denom = facen[0]*ptray[0] + facen[1]*ptray[1] + facen[2]*ptray[2];
    if (doDebug)
        trx.debug << "\n" << colA( "denom" ) << xvalue( denom );

    if (denom == 0) {
        if (doDebug) {
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "denom != 0" )
                      << xlunindent << "\n";
        }
        return false;
    }

    float mu = -( dist + DotProduct( facen, trx.start )) / denom;
    if (doDebug)
        trx.debug << "\n" << colA( "mu" ) << xvalue( mu );

    // Prune if no intersect point.
    if (mu < 0 || mu > 1) {
        if (doDebug) {
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "no intersect" )
                      << xlunindent << "\n";
        }
        return false;
    }

    VectorMA( trx.start, mu, ptray, rpos );

    // Compute intersect point is within quad bounds.
    bool hit = true;
    if (rpos[iboundx] < qmins[iboundx] || rpos[iboundx] > qmaxs[iboundx])
        hit = false;
    else if (rpos[iboundy] < qmins[iboundy] || rpos[iboundy] > qmaxs[iboundy])
        hit = false;

    if (!hit) {
        if (doDebug) {
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "not inside face" )
                      << xlunindent << "\n";
        }
        return false;
    }

    vec3_t pv;
    VectorSubtract( rpos, trx.start, pv );
    rlen = VectorLength( pv );

    if (doDebug) {
        colA.color = xcdebugBOLD;
        trx.debug << "\n" << colA( "rlen" ) << xvalue( rlen )
                  << "\n" << colA( "rpos" ) << xvec3 ( rpos )
                  << xlunindent << "\n";
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
AlignedCuboidHV::doEntityCompute()
{
    // Store size -> origin2
    // Store center of aabb -> origin
    VectorSubtract( maxs, mins, _entity->s.origin2 );

    VectorCopy( _entity->s.origin2, _entity->s.origin );
    VectorScale( _entity->s.origin, 0.5f, _entity->s.origin );
    VectorAdd( _entity->s.origin, mins, _entity->s.origin );

    if ( _entity->r.currentOrigin[0] != _entity->s.origin[0] &&
         _entity->r.currentOrigin[1] != _entity->s.origin[1] &&
         _entity->r.currentOrigin[2] != _entity->s.origin[2] )
    {
        // Must set currentOrigin and re-link otherwise moving out of PVS will prune.
        VectorCopy( _entity->s.origin, _entity->r.currentOrigin );
        trap_LinkEntity( _entity );
    }
}
