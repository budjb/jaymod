#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

Database::Database( const string& filename, const string& key )
    : _filename ( filename )
    , _key      ( key )
{
}

///////////////////////////////////////////////////////////////////////////////

Database::~Database()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Database::close()
{
    _stream.close();
    _stream.clear();
}

///////////////////////////////////////////////////////////////////////////////

void
Database::logBegin( bool write, const string& filename )
{
    process.beginCriticalSection();

    _logbeginTimestamp = process.mstime();
    ostringstream msg;
    msg << (write ? "Writing: " : "Reading: ") << filename;
    trap_Printf( msg.str().c_str() );
}

///////////////////////////////////////////////////////////////////////////////

void
Database::logEnd( int num, const string& type )
{
    Process::mstime_t elapsed = process.mstime() - _logbeginTimestamp;
    ostringstream msg;
    msg << ", " << num << " " << type << " (" << elapsed << "ms)" << endl;
    trap_Printf( msg.str().c_str() );

    process.endCriticalSection();
}

///////////////////////////////////////////////////////////////////////////////

bool
Database::open( bool write, string& fname )
{
    _currentKeyValue.clear();

    const ios::openmode mode = write ? ios::out : ios::in;
    
    // Get paths
    char buffer[ MAX_CVAR_VALUE_STRING ];
    trap_Cvar_VariableStringBuffer( "fs_homepath", buffer, sizeof( buffer ));
    fname = buffer;
    fname += "/";

    trap_Cvar_VariableStringBuffer( "fs_game", buffer, sizeof( buffer ));
    fname += buffer;
    fname += "/";
    fname += _filename;

    // Open file
    _stream.clear();
    _stream.open( fname.c_str(), mode );
    if ( !_stream.rdstate() )
        return false;

    // Error
    ostringstream msg;
    msg.str( "" );
    msg << "-------" << endl
        << "------- WARNING: unable to open " << fname << " ." << endl
        << "------- Please verify file is available for access." << endl
        << "-------" << endl;
    trap_Printf( msg.str().c_str() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Database::parseData( map<string,string>& data )
{
    data.clear();

    string name;
    string value;

    if ( _currentKeyValue.length() ) {
        data[ _key ] = _currentKeyValue;
        _currentKeyValue.clear();
    }

    while ( !parsePair( name, value ) ) {
        if ( name == _key ) {
            _currentKeyValue = value;
            break;
        }
        data[ name ] = value;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
Database::parsePair( string& name, string& value )
{
    enum Mode { LTRIM, COMMENT, NAME, DELIM, VALUE };
    Mode mode = LTRIM;
    bool delimVisible = false;

    int visibleLength = 0;

    for (char c; !_stream.get(c).rdstate(); ) {
        switch (mode) {
        case LTRIM:
            switch (c) {
            case '\0':  // NULL
            case '\t':  // TAB
            case ' ':   // SPACE
            case '\n':  // NEWLINE
            case '\r':  // CARRIAGE-RETURN
                break;

            case '#':  // COMMENT
                mode = COMMENT;
                break;

            default:
                mode = NAME;
                name = tolower( c );
                break;
            }
            break;

        case COMMENT:
            switch (c) {
            case '\n':  // NEWLINE
            case '\r':  // CARRIAGE-RETURN
                mode = LTRIM;
                name.clear();
                break;

            default:
                break;
            }
            break;

        case NAME:
            switch (c) {
            case '\0':  // NULL
                break;

            case '\n':  // NEWLINE
            case '\r':  // CARRIAGE-RETURN
                mode = LTRIM;
                break;

            case '\t':  // TAB
            case ' ':   // SPACE
            case '=':   // DELIMITER
                mode = DELIM;
                delimVisible = false;
                break;

            default:
                name += tolower( c );
                break;
            }
            break;

        case DELIM:
            switch (c) {
            case '\n':  // NEWLINE
            case '\r':  // CARRIAGE-RETURN
                mode = LTRIM;
                name.clear();
                break;

            case '\0':  // NULL
            case '\t':  // TAB
            case ' ':   // SPACE
                break;

            case '=':  // DELIMITER
                if (delimVisible) {
                    mode = VALUE;
                    value = c;
                    visibleLength = value.length();
                }
                delimVisible = true;
                break;

            default:
                mode = VALUE;
                value = c;
                visibleLength = value.length();
                break;
            }
            break;

        case VALUE:
            switch (c) {
            case '\0':  // NULL
                break;

            case '\n':  // NEWLINE
            case '\r':  // CARRIAGE-RETURN
                if (visibleLength)
                    value.resize( visibleLength );
                return false;

            case '\t':  // TAB
            case ' ':   // SPACE
                value += ' ';
                break;

            default:
                value += c;
                visibleLength = value.length();
                break;
            }
            break;

        default:
            break;
        }
    }

    if (mode != VALUE)
        return true;

    if (visibleLength)
        value.resize( visibleLength );

    return false;
}
