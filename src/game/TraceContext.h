#ifndef GAME_TRACECONTEXT_H
#define GAME_TRACECONTEXT_H

///////////////////////////////////////////////////////////////////////////////

class Client;
class AbstractHitVolume;

class TraceContext
{
private:
    TraceContext(); // not permitted

    void dump( const string&, int );

public:
    TraceContext( const TraceContext& );
    TraceContext( gentity_t&, gentity_t&, int mask, const vec3_t, const vec3_t );
    ~TraceContext();

    bool resultIsPlayer();

    bool trace       ( const string& = "", int index = -1 );
    bool traceNoEnts ( const string& = "", int index = -1 );

    text::Buffer debug;

    const int          time;    // time for trace, useful for antilag
    gentity_t&         source;  // source of trace (eg. weapon or player)
    gentity_t&         actor;   // actor of trace (eg. player acting with an entity-weapon)
    Client* const      client;  // convenience pointer to source/actor if a player
    const int          mask;    // content mask
    const vec3_t       start;   // start point of ray to trace
    const vec3_t       end;     // end point of ray to trace
    trace_t            data;    // result: standard trace data
    vec3_t             fpos;    // result: final position (hit point or end of ray)
    float              flen;    // result: final length of ray (up to hit point or end of ray)
    AbstractHitVolume* hitvol;  // result: final hit-volume if resultIsPlayer == true
    bool               water;   // result: true if traced through water
    trace_t            wdata;   // result: water standard trace data
    vec3_t             wpos;    // result: water hit position
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_TRACECONTEXT_H
