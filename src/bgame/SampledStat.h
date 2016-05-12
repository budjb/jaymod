#ifndef BGAME_SAMPLEDSTAT_H
#define BGAME_SAMPLEDSTAT_H

///////////////////////////////////////////////////////////////////////////////

class SampledStat
{
public:
    typedef Process::mstime_t mstime_t;

private:
    typedef struct sample_s {
        float    value;
        mstime_t elapsed;
        mstime_t time;
    } sample_t;

private:
    SampledStat(); // not permitted

    mstime_t _period;
    float    _fvalue;
    int      _ivalue;
    float    _avg;

    mstime_t        _lastSampleTime;
    list<sample_t*> _samples;
    sample_t        _samplesTotal;

public:
    SampledStat( mstime_t );  // sample period in milliseconds
    ~SampledStat();

    float avg    ( );  // reeturns rate/s
    void  sample ( float );
    void  sample ( int   );
};

///////////////////////////////////////////////////////////////////////////////

namespace stats {

///////////////////////////////////////////////////////////////////////////////

extern SampledStat entitySpawn;
extern SampledStat entityFree;
extern SampledStat frame;

///////////////////////////////////////////////////////////////////////////////

} // namespace stats

#endif // BGAME_SAMPLEDSTAT_H
