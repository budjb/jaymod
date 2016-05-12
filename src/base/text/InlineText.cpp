#include <base/public.h>
#include <base/text/impl.h>

namespace text {

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText()
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( const InlineText& ref )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator=( ref );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( const ColorManipulator& color_ )
    : color     ( color_ )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( bool v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( float v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( double v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( int8 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( int16 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( int32 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( int64 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( uint8 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( uint16 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( uint32 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( uint64 v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( const void* v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( const char* v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineText::InlineText( const string& v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{
    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

#if defined( JAYMOD_OSX )
InlineText::InlineText( size_t v )
    : color     ( xcnone )
    , flags     ( ios::fixed )
    , precision ( 6 )
    , width     ( 0 )
    , value     ( _value )
{   
    operator()( v );
}   
#endif // JAYMOD_OSX

///////////////////////////////////////////////////////////////////////////////

InlineText::~InlineText()
{
}

///////////////////////////////////////////////////////////////////////////////

void
InlineText::compute( const string& v )
{
    string sub;
    sub.append( prefix );
    sub.append( v );
    sub.append( suffix );

    _oss.str( "" );
    _oss.flags( flags );
    _oss.width( width );
    _oss.precision( precision );
    _oss << sub;
    _value = _oss.str();
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator=( const InlineText& ref )
{
    _value        = ref.value;
    color         = ref.color;
    flags         = ref.flags;
    precision     = ref.precision;
    width         = ref.width;
    prefix        = ref.prefix;
    prefixOutside = ref.prefixOutside;
    suffix        = ref.suffix;
    suffixOutside = ref.suffixOutside;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( bool v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( float v )
{
    ostringstream vss;
    vss.flags( flags );
    vss.precision( precision );

    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( double v )
{
    ostringstream vss;
    vss.flags( flags );
    vss.precision( precision );

    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( int8 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( int16 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( int32 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( int64 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( uint8 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( uint16 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( uint32 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( uint64 v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( const void* v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( const char* v )
{
    compute( string(v) );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

InlineText&
InlineText::operator()( const string& v )
{
    compute( v );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

#if defined( JAYMOD_OSX )
InlineText&
InlineText::operator()( size_t v )
{
    ostringstream vss;
    vss << v;
    compute( vss.str() );
    return *this;
}
#endif // JAYMOD_OSX

///////////////////////////////////////////////////////////////////////////////

InlineVec3::InlineVec3()
    : InlineText ( xcvalue )
{
    precision = 2;
    width     = 9;
    prefix    = "[ ";
    suffix    = " ]";
}

///////////////////////////////////////////////////////////////////////////////

InlineVec3::InlineVec3( const InlineVec3& ref )
    : InlineText ( xcvalue )
{
    operator=( ref );
}

///////////////////////////////////////////////////////////////////////////////

InlineVec3::InlineVec3( const float* v )
    : InlineText ( xcvalue )
{
    precision = 2;
    width     = 9;
    prefix    = "[ ";
    suffix    = " ]";

    operator()( v );
}

///////////////////////////////////////////////////////////////////////////////

InlineVec3::~InlineVec3()
{
}

///////////////////////////////////////////////////////////////////////////////

InlineVec3&
InlineVec3::operator()( const float* v )
{
    ostringstream vss;
    vss.flags( flags );
    vss.precision( precision );

    vss << setw( width ) << v[0] << ", "
        << setw( width ) << v[1] << ", "
        << setw( width ) << v[2];
    InlineText::operator()( vss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace text
