#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Uptime::Uptime()
    : AbstractBuiltin( "uptime", true )
{
    __usage << xvalue( "!" + _name );
    __descr << "Displays how long the server has been running.";
}

///////////////////////////////////////////////////////////////////////////////

Uptime::~Uptime()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Uptime::doExecute( Context& txt )
{
    if (txt._args.size() > 1)
        return PA_USAGE;

    Buffer buf;
    buf << _name << ": "
        << xvalue( str::toStringSecondsRemaining( time(NULL) - sv_uptimeStamp.integer, true ))
        << '.';
    printChat( txt._client, buf );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
