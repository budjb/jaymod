#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Lock::Lock()
    : AbstractBuiltin( "lock" )
{
    __usage << xvalue( "!" + _name )
            << " <" << xvalue( "r" )
            << '|' << xvalue( "b" )
            << '|' << xvalue( "s" )
            << '|' << xvalue( "all" )
            << '>';

    __descr << "Lock a team so players cannot enter it.";
}

///////////////////////////////////////////////////////////////////////////////

Lock::~Lock()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Lock::doExecute( Context& txt )
{
    return lockTeams( txt._client, txt._args, txt._ebuf, true );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
