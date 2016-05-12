#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

ResetmyXp::ResetmyXp()
    : AbstractBuiltin( "resetmyxp" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Completely reset your own XP.";
}

///////////////////////////////////////////////////////////////////////////////

ResetmyXp::~ResetmyXp()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
ResetmyXp::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    // bail if console
    if (txt._user == User::CONSOLE) {
        txt._ebuf << "You are on the console.";
        return PA_ERROR;
    }

    txt._client->xpReset();

    Buffer buf;
    buf << _name << ": " << xvalue( txt._user.namex ) << "'s XP has been reset.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
