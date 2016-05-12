#include <bgame/impl.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

Client*
__clientForContext( gentity_t& source, gentity_t& actor )
{
    if (source.client)
        return &g_clientObjects[source.s.number];

    if (actor.client)
        return &g_clientObjects[actor.s.number];

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int
__timeForContext( gentity_t& source, gentity_t& actor )
{
    if (source.client) {
        usercmd_t cmd;
        trap_GetUsercmd( source.s.number, &cmd );
        return cmd.serverTime;
    }

    if (actor.client) {
        usercmd_t cmd;
        trap_GetUsercmd( actor.s.number, &cmd );
        return cmd.serverTime;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

using namespace text;

///////////////////////////////////////////////////////////////////////////////

TraceContext::TraceContext( const TraceContext& obj )
    : debug  ( xcdebug )
    , time   ( obj.time )
    , source ( obj.source )
    , actor  ( obj.actor )
    , client ( obj.client )
    , mask   ( obj.mask )
    , start  ( )  // array initialization not permitted by ISO C++
    , end    ( )  // array initialization not permitted by ISO C++
    , hitvol ( 0 )
    , water  ( false )
{
    memcpy( const_cast<vec3_t*>(&start), obj.start, sizeof(start) );
    memcpy( const_cast<vec3_t*>(&end), obj.end, sizeof(end) );
    memset( &wdata, 0, sizeof(wdata) );
    memset( &wpos, 0, sizeof(wpos) );
}

///////////////////////////////////////////////////////////////////////////////

TraceContext::TraceContext(
    gentity_t&   source_,
    gentity_t&   actor_,
    int          mask_,
    const vec3_t start_,
    const vec3_t end_ )

    : debug  ( xcdebug )
    , time   ( __timeForContext( source_, actor_ ))
    , source ( source_ )
    , actor  ( actor_ )
    , client ( __clientForContext( source_, actor_ ))
    , mask   ( mask_ )
    , start  ( )  // array initialization not permitted by ISO C++
    , end    ( )  // array initialization not permitted by ISO C++
    , hitvol ( 0 )
    , water  ( false )
{
    memcpy( const_cast<vec3_t*>(&start), start_, sizeof(start) );
    memcpy( const_cast<vec3_t*>(&end), end_, sizeof(end) );
    memset( &wdata, 0, sizeof(wdata) );
    memset( &wpos, 0, sizeof(wpos) );
}

///////////////////////////////////////////////////////////////////////////////

TraceContext::~TraceContext()
{
}

///////////////////////////////////////////////////////////////////////////////

void
TraceContext::dump( const string& name, int index )
{
    debug << xheader( name );
    if (index > -1)
        debug << "[" << xheader( index ) << "]";

    InlineText colA;

    colA.flags |= ios::left;
    colA.width  = 14;
    colA.suffixOutside = " = ";

    debug << xlindent
        << "\n" << colA( "source"         ) << xvalue ( source.s.number )
        << "\n" << colA( "source.origin"  ) << xvec3  ( source.r.currentOrigin )
        << "\n" << colA( "actor"          ) << xvalue ( actor.s.number )
        << "\n" << colA( "actor.origin"   ) << xvec3  ( actor.r.currentOrigin )
        << "\n" << colA( "content mask"   ) << xvalue ( str::toHexString( mask ))
        << "\n" << colA( "start"          ) << xvec3  ( start )
        << "\n" << colA( "end"            ) << xvec3  ( end )
        << "\n" << colA( "d.startsolid"   ) << xvalue ( bool(data.startsolid) )
        << "\n" << colA( "d.allsolid"     ) << xvalue ( bool(data.allsolid) )
        << "\n" << colA( "d.fraction"     ) << xvalue ( data.fraction )
        << "\n" << colA( "d.surfaceFlags" ) << xvalue ( str::toHexString( data.surfaceFlags ))
        << "\n" << colA( "d.contents"     ) << xvalue ( str::toHexString( data.contents ))
        << "\n" << colA( "d.endpos"       ) << xvec3  ( data.endpos )
        << "\n" << colA( "d.plane.dist"   ) << xvalue ( data.plane.dist )
        << "\n" << colA( "d.plane.normal" ) << xvec3  ( data.plane.normal )
        << "\n" << colA( "d.plane.type"   ) << xvalue ( data.plane.type )
        << "\n" << colA( "d.entityNum"    ) << xvalue ( data.entityNum )
        << "\n" << colA( "flen"           ) << xvalue ( flen )
        << "\n" << colA( "fpos"           ) << xvec3  ( fpos )
        << xlunindent << "\n";
}

///////////////////////////////////////////////////////////////////////////////

bool
TraceContext::resultIsPlayer()
{
    return (data.entityNum > -1 && data.entityNum < MAX_CLIENTS);
}

///////////////////////////////////////////////////////////////////////////////

bool
TraceContext::trace( const string& name, int index )
{
    trap_Trace( &data, start, 0, 0, end, ENTITYNUM_NONE, mask );
    trap_Trace( &wdata, start, 0, 0, end, ENTITYNUM_NONE, mask | MASK_WATER );
    water = data.fraction != wdata.fraction && wdata.fraction != 1.0f;

    // Update final pos/len.
    vec3_t v;
    VectorSubtract( data.endpos, start, v );
    flen = VectorLength( v );
    VectorCopy( data.endpos, fpos );

    if (cvars::g_hitmodeDebug.ivalue & AbstractHitModel::DEBUG_TRAY)
        dump( JAYMOD_FUNCTION, index );

    return (data.fraction != 1.0f);
}

///////////////////////////////////////////////////////////////////////////////

bool
TraceContext::traceNoEnts( const string& name, int index )
{
    trap_TraceNoEnts( &data, start, 0, 0, end, 0, mask );
    trap_Trace( &wdata, start, 0, 0, end, ENTITYNUM_NONE, mask | MASK_WATER );
    water = data.fraction != wdata.fraction && wdata.fraction != 1.0f;

    // Update final pos/len.
    vec3_t v;
    VectorSubtract( data.endpos, start, v );
    flen = VectorLength( v );
    VectorCopy( data.endpos, fpos );

    if (cvars::g_hitmodeDebug.ivalue & AbstractHitModel::DEBUG_TRAY)
        dump( JAYMOD_FUNCTION, index );

    return (data.fraction != 1.0f);
}
