#include <base/public.h>

namespace str {

///////////////////////////////////////////////////////////////////////////////

void
concatArgs( const vector<string>& args, string& out, vector<string>::size_type offset )
{
    out.clear();

    const vector<string>::size_type max = args.size();
    for ( vector<string>::size_type i = offset; i < max; i++ ) {
        if (i > offset)
            out += ' ';
        out += args[i];
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
isIndex( const string& value )
{
    if (value.empty())
        return false;

    const string::size_type max = value.length();
    for ( string::size_type i = 0; i < max; i++ ) {
        const char& c = value[i];
        if (c < '0' || c > '9')
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

string&
toLower( string& value )
{
    const string::size_type max = value.length();
    for ( string::size_type i = 0; i < max; i++ )
        value[i] = ::tolower( value[i] );
    return value;
}

///////////////////////////////////////////////////////////////////////////////

string
toLowerCopy( const string& value )
{
    string result = value;
    return toLower( result );
}

///////////////////////////////////////////////////////////////////////////////

string&
toUpper( string& value )
{
    const string::size_type max = value.length();
    for ( string::size_type i = 0; i < max; i++ )
        value[i] = ::toupper( value[i] );
    return value;
}

///////////////////////////////////////////////////////////////////////////////

string
toUpperCopy( const string& value )
{
    string result = value;
    return toUpper( result );
}

///////////////////////////////////////////////////////////////////////////////

string::size_type
iFind( string needle, string haystack, string::size_type index )
{
    // Convert these to lower case
    toLower(needle);
    toLower(haystack);

    // Do the find
    return haystack.find(needle, index);
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
