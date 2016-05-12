#include <bgame/impl.h>

namespace str {

///////////////////////////////////////////////////////////////////////////////

string
toDropMessage( bool duringConnect, const text::Buffer& msg, const string& action )
{
    string result;
    return toDropMessage( result, duringConnect, msg, action );
}

///////////////////////////////////////////////////////////////////////////////

string&
toDropMessage( string& out, bool duringConnect, const text::Buffer& msg, const string& action )
{
    using namespace text;

    // setup device with general characteristics of defacto UI popup
    ETDevice dev( 37, 14, 2 );

    Buffer buf;

    /* If the client is dropped during engine call G_DROP_CLIENT then
     * the engine automatically prefixes the message with:
     *
     *    Server Disconnected -
     *
     * But if the client is dropped during ClientConnect semantics,
     * then there is no prefix. Therefore we will supply a similar
     * prefix which indicates this happened during connection.
     */
    if (duringConnect)
        buf << "Connection Refused - ";

    buf << xnone( JAYMOD_namex );

    if (!action.empty())
        buf << '\n' << xvalue( action );

    buf << '\n'
        << msg;

    if (cvars::g_protestMessage.svalue[0])
        buf << "\n\n" << cvars::g_protestMessage.svalue;

    // convert device output lines to multi-line string
    ostringstream oss;
    list<string> output;
    dev.format( buf, output );
    const list<string>::iterator end = output.end();
    for ( list<string>::iterator it = output.begin(); it != end; it++ )
        oss << *it;

    out = oss.str();
    return out;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
