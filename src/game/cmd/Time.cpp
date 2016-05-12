#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Time::Time()
    : AbstractBuiltin( "time", true )
{
    __usage << xvalue( "!" + _name );
    __descr << "Shows the current local (server) time.";
}

///////////////////////////////////////////////////////////////////////////////

Time::~Time()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Time::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    time_t now;
    if (!time( &now )) {
        txt._ebuf << "Unable to determine system time.";
        return PA_ERROR;
    }

    tm* lt = localtime( &now );
    char s[32];
    strftime( s, sizeof(s), "%I:%M%p %Z", lt );

    Buffer buf;
    buf << _name << ": " << xvalue( s );
    printChat( txt._client, buf );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
