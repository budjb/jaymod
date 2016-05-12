#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unlock::Unlock()
    : AbstractBuiltin( "unlock" )
{
    __usage << xvalue( "!" + _name )
            << " <" << xvalue( "r" )
            << '|' << xvalue( "b" )
            << '|' << xvalue( "s" )
            << '|' << xvalue( "all" )
            << '>';

    __descr << "Unlock a team so players can enter it again.";
}

///////////////////////////////////////////////////////////////////////////////

Unlock::~Unlock()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unlock::doExecute( Context& txt )
{
    return lockTeams( txt._client, txt._args, txt._ebuf, false );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
