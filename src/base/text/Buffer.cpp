#include <base/public.h>
#include <base/text/impl.h>

namespace text {

///////////////////////////////////////////////////////////////////////////////

Buffer::Buffer( const ColorManipulator& manip )
    : _data     ( NULL )
    , _length   ( 0 )
    , _numLines ( 1 )
    , _size     ( 1024 )
    , data      ( _data )
    , length    ( _length )
    , numLines  ( _numLines )
{
    _data = new uint8[ _size ];
    *this << manip;
}

///////////////////////////////////////////////////////////////////////////////

Buffer::Buffer( const uint8* data_, uint32 size_, const ColorManipulator& manip )
    : _data     ( NULL )
    , _length   ( size_ + 1 )
    , _numLines ( 1 )
    , _size     ( size_ + 1 )
    , data      ( _data )
    , length    ( _length )
    , numLines  ( _numLines )
{
    _data = new uint8[ _size ];
    _data[0] = manip.code;
    memcpy( _data + 1, data_, _size - 1 );
}

///////////////////////////////////////////////////////////////////////////////

Buffer::~Buffer()
{
    delete[] _data;
}

///////////////////////////////////////////////////////////////////////////////

void
Buffer::append( uint8 value )
{
    if (_size - _length < 1) {
        _size = uint32((_size+1) * 1.4);
        uint8* p = new uint8[_size];
        memcpy( p, _data, _length );
        delete[] _data;
        _data = p;
    }

    _data[_length++] = value;
}

///////////////////////////////////////////////////////////////////////////////

void
Buffer::append( const string& value )
{
    /* It is safe to assume we will require at max len because upon encoding,
     * from a typical ET color-string, we either will be same size if no color
     * codes exist or less because our encoding is more efficient.
     */
    string::size_type len = value.length();
    if (_size - _length < len) {
        _size = uint32((_size+len) * 1.4);
        uint8* p = new uint8[_size];
        memcpy( p, _data, _length );
        delete[] _data;
        _data = p;
    }

    enum { CHAR, ESCAPE } mode = CHAR;

    for ( string::size_type i = 0; i < len; i++ ) {
        const char c = value[i];
        switch (mode) {
            default:
            case CHAR:
                switch (c) {
                    case '\n':
                        _data[_length++] = MANIP_NEWLINE;
                        _numLines++;
                        break;

                    case ' ':
                        _data[_length++] = c;
                        break;

                    case '^':
                        mode = ESCAPE;
                        break;

                    default:
                        if (isgraph(c)) {
                            if (c > 31)
                                _data[_length++] = c;
                            else
                                _data[_length++] = '.';
                        }
                        break;
                }
                break;

            case ESCAPE:
                switch (c) {
                    case '^':
                        _data[_length++] = '^';
                        break;

                    default:
                        if (isgraph(c) && c > 0)
                            _data[_length++] = MANIP_COLOR_BLACK + ((c - '0') & 31);
                        mode = CHAR;
                        break;
                }
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Buffer::dump()
{
    ostringstream out;
    out << "BUFFER: length=" << _length << " size=" << _size << "\n";

    out << noshowbase << hex;
    for (uint32 i = 0; i < _length; i++) {
        out << ' ' << setw(2) << (int)_data[i];
    }

    out << "\n";

    cout << out.str();
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( const Buffer& src )
{
    if (_size - _length < src.length) {
        _size = uint32((_size+src.length) * 1.4);
        uint8* p = new uint8[_size];
        memcpy( p, _data, _length );
        delete[] _data;
        _data = p;
    }

    memcpy( _data + _length, src.data, src.length );
    _length += src.length;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( const InlineText itext )
{
    append( MANIP_CPUSH );
    append( itext.prefixOutside );

    append( MANIP_CPUSH );
    append( itext.color.code );
    append( itext.value );
    append( MANIP_CPOP );

    append( itext.suffixOutside );
    append( MANIP_CPOP );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( const Manipulator& manip )
{
    append( manip.code );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( char value )
{
    append( string( 1, value ));
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( int value )
{
    ostringstream oss;
    oss << value;
    append( oss.str() );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( const char* value )
{
    append( string( value ));
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Buffer&
Buffer::operator<<( string value )
{
    append( value );
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

void
Buffer::reset( const ColorManipulator& manip )
{
    _length   = 0;
    _numLines = 1;

    *this << manip;
}

///////////////////////////////////////////////////////////////////////////////

void
Buffer::toCommands( const string& cmd, list<string>& commands, int maxBuffer ) const
{
    commands.clear();

    const int encMax = maxBuffer - cmd.length() - 1;
    const int textMax = encMax * 3 / 4 - 1;

    const int encsiz = encMax + 1;
    char* enc = (char*)malloc( encsiz );

    string cmdAccumulate = cmd;
    const string::size_type idx = cmdAccumulate.length() - 1;
    cmdAccumulate[idx] = toupper( cmdAccumulate[idx] );

    const uint8* const pmax = _data + _length;
    for ( const uint8* p = _data; p < pmax; p += textMax ) {
        commands.push_back( "" );
        string& s = commands.back();

        const int remain = pmax - p;
        int segsiz;
        if (remain > textMax) {
            s = cmdAccumulate;
            segsiz = textMax;
        }
        else {
            s = cmd;
            segsiz = remain;
        }

        s += " ";

        memset( enc, 0, encsiz );
        base64::encode( p, segsiz, enc, encsiz );

        s += enc;
    }

    free( enc );
}

///////////////////////////////////////////////////////////////////////////////

bool
Buffer::toReport( vector<Buffer*>& report, uint32 maxLines, uint32 maxPages ) const
{
    const vector<Buffer*>::size_type max = report.size();
    for ( vector<Buffer*>::size_type i = 0; i < max; i++ )
        delete report[i];
    report.clear();

    Buffer buf;
    const uint8* const pmax = _data + _length;
    const uint8* mark = _data;
    uint32 markLines = 0;
    for ( const uint8 *p = _data; p < pmax; p++ ) {
        if (*p != MANIP_NEWLINE)
            continue;

        if (++markLines < maxLines)
            continue;

        report.push_back( new Buffer( mark, p-mark, _data[0] ));
        mark = p + 1;
        markLines = 0;
    }

    if (mark < pmax && report.size() < maxPages)
        report.push_back( new Buffer( mark, pmax-mark, _data[0] ));

    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace text
