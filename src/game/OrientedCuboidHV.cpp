#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

OrientedCuboidHV::OrientedCuboidHV( zone_t zone_, AbstractHitModel& hitModel_, scope_t scope_ )
    : AbstractHitVolume ( TYPE_ORIENTED_CUBOID, zone_, ET_ORIENTED_HIT_VOLUME, hitModel_, scope_ )
{
    memset( origin, 0, sizeof(origin) );
    memset( scale, 0, sizeof(scale) );
    memset( axis, 0, sizeof(axis) );
    memset( coords, 0, sizeof(coords) );
}

///////////////////////////////////////////////////////////////////////////////

OrientedCuboidHV::~OrientedCuboidHV()
{
}

///////////////////////////////////////////////////////////////////////////////

OrientedCuboidHV&
OrientedCuboidHV::operator=( const OrientedCuboidHV& obj )
{
    AbstractHitVolume::operator=( obj );

    memcpy( origin, obj.origin, sizeof(origin ));
    memcpy( scale, obj.scale, sizeof(scale ));
    memcpy( axis, obj.axis, sizeof(axis ));
    memcpy( coords, obj.coords, sizeof(coords ));

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool
OrientedCuboidHV::castRay( TraceContext& trx, vec3_t& rpos, float& rlen )
{
    vec3_t tmppos;
    float  tmplen;

    // SOUTH-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "SOUTH (tri-0)", coords[0], coords[1], coords[2] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "SOUTH (tri-1)", coords[2], coords[3], coords[0] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // NORTH-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "NORTH (tri-0)", coords[4], coords[5], coords[6] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "NORTH (tri-1)", coords[6], coords[7], coords[4] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // WEST-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "WEST (tri-0)", coords[3], coords[2], coords[6] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "WEST (tri-1)", coords[6], coords[7], coords[3] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // EAST-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "EAST (tri-0)", coords[0], coords[4], coords[5] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "EAST (tri-1)", coords[5], coords[1], coords[0] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // TOP-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "TOP (tri-0)", coords[1], coords[5], coords[6] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "TOP (tri-1)", coords[6], coords[2], coords[1] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    // BOTTOM-FACE
    if (castRayTriangle( trx, tmppos, tmplen, "BOTTOM (tri-0)", coords[0], coords[4], coords[7] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }
    if (castRayTriangle( trx, tmppos, tmplen, "BOTTOM (tri-1)", coords[7], coords[3], coords[0] )) {
        VectorCopy( tmppos, rpos );
        rlen = tmplen;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
OrientedCuboidHV::castRayTriangle(
    TraceContext&  trx,
    vec3_t&        rpos,
    float&         rlen,
    const string&  name,
    const vec3_t&  p0,
    const vec3_t&  p1,
    const vec3_t&  p2 )
{
    vec3_t e1;
    vec3_t e2;
    VectorSubtract( p1, p0, e1 );
    VectorSubtract( p2, p0, e2 );

    // Compute directional ray.
    vec3_t dray;
    VectorSubtract( trx.end, trx.start, dray );

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
            << "\n" << colA( "tri[0]"   ) << xvec3( p0 )
            << "\n" << colA( "tri[1]"   ) << xvec3( p1 )
            << "\n" << colA( "tri[2]"   ) << xvec3( p2 )
            << "\n" << colA( "dray"     ) << xvec3( dray );
    }

    vec3_t h;
    CrossProduct( dray, e2, h );
    float a = DotProduct( e1, h );

    if (doDebug)
        trx.debug << "\n" << colA( "a" ) << xvalue( a );

    if (a == 0.0f) {
        if (doDebug)
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "a is 0" ) << xlunindent << "\n";
        return false;
    }

    float f = 1 / a;
    vec3_t s;
    VectorSubtract( trx.start, p0, s );

    float u = f * DotProduct( s, h );

    if (doDebug)
        trx.debug << "\n" << colA( "u" ) << xvalue( u );

    if (u < 0.0f || u > 1.0f) {
        if (doDebug)
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "u out of range" ) << xlunindent << "\n";
        return false;
    }

    vec3_t q;
    CrossProduct( s, e1, q );
    float v = f * DotProduct( dray, q );

    if (doDebug)
        trx.debug << "\n" << colA( "v" ) << xvalue( v );

    if (v < 0.0f || u+v > 1.0f) {
        if (doDebug)
            trx.debug << "\n" << colA( "pruned" ) << xvalue( "v out of range" ) << xlunindent << "\n";
        return false;
    }

    float t = f * DotProduct( e2, q );

    if (doDebug)
        trx.debug << "\n" << colA( "t" ) << xvalue( t );

    if (t == 0.0f)
        return false;

    rlen = t * VectorLength( dray ); 
    VectorMA( trx.start, rlen, dray, rpos );

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
OrientedCuboidHV::doEntityCompute()
{
    VectorCopy( origin, _entity->s.origin );
    VectorCopy( scale, _entity->s.origin2 );
    AxisToAngles( axis, _entity->s.angles );

    if ( _entity->r.currentOrigin[0] != _entity->s.origin[0] &&
         _entity->r.currentOrigin[1] != _entity->s.origin[1] &&
         _entity->r.currentOrigin[2] != _entity->s.origin[2] )
    {
        // Must set currentOrigin and re-link otherwise moving out of PVS will prune.
        VectorCopy( _entity->s.origin, _entity->r.currentOrigin );
        trap_LinkEntity( _entity );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
OrientedCuboidHV::reorient()
{
    VectorCopy( origin, mins );
    VectorCopy( origin, maxs );

    VectorCopy( origin, coords[0] );

    VectorCopy( coords[0], coords[1] );
    VectorMA( coords[1], scale[2], axis[2], coords[1] );

    VectorCopy( coords[1], coords[2] );
    VectorMA( coords[2], scale[1], axis[1], coords[2] );

    VectorCopy( coords[0], coords[3] );
    VectorMA( coords[3], scale[1], axis[1], coords[3] );

    VectorCopy( coords[0], coords[4] );
    VectorMA( coords[4], scale[0], axis[0], coords[4] );

    VectorCopy( coords[1], coords[5] );
    VectorMA( coords[5], scale[0], axis[0], coords[5] );

    VectorCopy( coords[2], coords[6] );
    VectorMA( coords[6], scale[0], axis[0], coords[6] );

    VectorCopy( coords[3], coords[7] );
    VectorMA( coords[7], scale[0], axis[0], coords[7] );

    for (int i = 0; i < 8; i++) {
        vec3_t& v = coords[i];

        if (v[0] < mins[0])
            mins[0] = v[0];
        else if (v[0] > maxs[0])
            maxs[0] = v[0];

        if (v[1] < mins[1])
            mins[1] = v[1];
        else if (v[1] > maxs[1])
            maxs[1] = v[1];

        if (v[2] < mins[2])
            mins[2] = v[2];
        else if (v[2] > maxs[2])
            maxs[2] = v[2];
    }
}
