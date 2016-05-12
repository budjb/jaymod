#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

FTime::FTime()
    : AbstractBuiltin( "ftime" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Shows the current Central Time in a FUN way.";
}

///////////////////////////////////////////////////////////////////////////////

FTime::~FTime()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
FTime::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    const time_t now = time( NULL );
    char s[32];
    strftime( s, sizeof(s), "%I:%M:%S %p", localtime( &now ));

    stringstream str;
    str << setw(11) << right << s;

    Buffer buf;

    // Print the little guy
    buf << "       ^7'''                      ^08888888888   888888888888  88\n"
        << "      ^o(o o)       ^xT^fIME ^xF^fOR ^xF^fUN  ^088                   .8P'  88\n"
        << "^1+^4=^oo00^4==^o(_)^4======^1+               ^088                  d8'    88\n"
        << "^4|  ^7" << str.str() << "  ^4|  ^08b       d8  88a8PPPP8b,       ,8P'     88\n"
        << "^1+^4===========^oo00^4=^1+  ^0'8b     d8'  PP'      8b      d8'       88\n"
        << "     ^6|__|__|        ^0'8b   d8'            d8    ,8P'        ''\n"
        << "      ^o|| ||          ^0'8b,d8'    Y8a     a8P   d8'          aa\n"
        << "     ^ooo0 0oo           ^0'8'       'Y88888P'   8P'           88\n";

    printChat( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
