#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace str {

///////////////////////////////////////////////////////////////////////////////

PairDump::PairDump( string header_ )
    : _header ( header_ )
{
    clear();
}

///////////////////////////////////////////////////////////////////////////////

PairDump::~PairDump()
{
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, bool value )
{
    Data d = { name, value ? "true" : "false" };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, int32 value )
{
    ostringstream oss;
    oss << value;
    Data d = { name, oss.str() };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, uint32 value )
{
    ostringstream oss;
    oss << value;
    Data d = { name, oss.str() };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, float value )
{
    ostringstream oss;
    oss << value;
    Data d = { name, oss.str() };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, double value )
{
    ostringstream oss;
    oss << value;
    Data d = { name, oss.str() };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, string value )
{
    Data d = { name, value };
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addEntry( string name, const vec3_t& v )
{
    Data d = { name, str::toString( v )};
    _section->entries.push_back( d );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::addSection( string name )
{
    Section s = { name };
    _sections.push_back( s );
    _section = &_sections.back();
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::clear()
{
    _sections.clear();
    _sections.push_back( Section() );
    _section = &_sections.back();
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::dump( ostream& out )
{
    // save stream settings
    const streamsize    precision = out.precision();
    const ios::fmtflags flags     = out.flags();

    out.flags( ios::fixed );
    out.precision( 2 );

    out << _header << ": DUMP";

    const list<Section>::iterator end = _sections.end();
    for ( list<Section>::iterator it = _sections.begin(); it != end; it++ ) {
        Section& s = *it;

        if (s.name.length())
            out << "\n  ---- " << s.name << " ----";

        size_t len = 4;
        const list<Data>::iterator end2 = s.entries.end();
        for ( list<Data>::iterator it2 = s.entries.begin(); it2 != end2; it2++ ) {
            Data& d = *it2;
            if (d.name.length() > len)
                len = d.name.length();
        }

        for ( list<Data>::iterator it2 = s.entries.begin(); it2 != end2; it2++ ) {
            Data& d = *it2;
            out << "\n  " << setw(len) << left << d.name << " = " << d.value;
        }
    }

    out << endl;

    // restore stream settings
    out.precision( precision );
    out.flags( flags );
}

///////////////////////////////////////////////////////////////////////////////

void
PairDump::dump( const PairDump& other, ostream& out )
{
    // index values in other dump
    typedef map<string,const Data*> Map;
    Map map;

    {
        const list<Section>::const_iterator end = other._sections.end();
        for ( list<Section>::const_iterator it = other._sections.begin(); it != end; it++ ) {
            const Section& s = *it;

            const list<Data>::const_iterator end2 = s.entries.end();
            for ( list<Data>::const_iterator it2 = s.entries.begin(); it2 != end2; it2++ ) {
                const Data& d = *it2;
                map[ s.name + "/" + d.name ] = &d;
            }
        }
    }

    // save stream settings
    const streamsize    precision = out.precision();
    const ios::fmtflags flags     = out.flags();

    out.flags( ios::fixed );
    out.precision( 2 );

    out << _header << ": DELTA DUMP";

    const list<Section>::iterator end = _sections.end();
    for ( list<Section>::iterator it = _sections.begin(); it != end; it++ ) {
        Section& s = *it;

        if (s.name.length())
            out << "\n  ---- " << s.name << " ----";

        size_t len = 4;
        const list<Data>::iterator end2 = s.entries.end();
        for ( list<Data>::iterator it2 = s.entries.begin(); it2 != end2; it2++ ) {
            Data& d = *it2;
            if (d.name.length() > len)
                len = d.name.length();
        }

        for ( list<Data>::iterator it2 = s.entries.begin(); it2 != end2; it2++ ) {
            Data& d = *it2;
            out << "\n  " << setw(len) << left << d.name << " = " << d.value;

            Map::iterator found = map.find( s.name + "/" + d.name );
            if (found == map.end())
                continue;

            const Data& d2 = *found->second;
            if (d.value == d2.value)
                continue;

            out << " --> " << d2.value;
        }
    }

    out << endl;

    // restore stream settings
    out.precision( precision );
    out.flags( flags );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
