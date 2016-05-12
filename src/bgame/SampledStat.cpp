#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

SampledStat::SampledStat( mstime_t period_ )
    : _period         ( period_ )
    , _fvalue         ( 0.0f )
    , _ivalue         ( 0 )
    , _avg            ( 0.0f )
    , _lastSampleTime ( 0 )
{
    memset( &_samplesTotal, 0, sizeof(_samplesTotal) );
}

///////////////////////////////////////////////////////////////////////////////

SampledStat::~SampledStat()
{
    const list<sample_t*>::iterator end = _samples.end();
    for ( list<sample_t*>::iterator it = _samples.begin(); it != end; it++ )
        delete *it;
}

///////////////////////////////////////////////////////////////////////////////

float
SampledStat::avg()
{
    const mstime_t now = process.mstime();
    if (now - _lastSampleTime > 1000)
        sample( 0 );

    return _avg;
}

///////////////////////////////////////////////////////////////////////////////

void
SampledStat::sample( float f )
{
    _fvalue = f;
    _ivalue = int(f);

    const mstime_t now = process.mstime();

    sample_t& s = *new sample_t;
    s.value   = _fvalue;
    s.elapsed = now - _lastSampleTime;
    s.time    = now;

    if (s.elapsed < 0)
        s.elapsed = 0;

    _lastSampleTime = now;

    _samples.push_front( &s );
    _samplesTotal.value   += s.value;
    _samplesTotal.elapsed += s.elapsed;

    const mstime_t oldest = now - _period;
    while (!_samples.empty()) {
        sample_t& back = *_samples.back();

        if (back.time > oldest)
            break;

        _samplesTotal.value   -= back.value;
        _samplesTotal.elapsed -= back.elapsed;

        _samples.pop_back();
        delete &back;
    }

    _avg = _samplesTotal.elapsed > 0
        ? _samplesTotal.value / float(_samplesTotal.elapsed / 1000.0f)
        : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

void
SampledStat::sample( int i )
{
    sample( float(i) );
}

///////////////////////////////////////////////////////////////////////////////

namespace stats {

///////////////////////////////////////////////////////////////////////////////

SampledStat entitySpawn ( 15*1000 );
SampledStat entityFree  ( 15*1000 );
SampledStat frame       (  5*1000 );

///////////////////////////////////////////////////////////////////////////////

} // namespace stats
