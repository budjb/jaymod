#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unpause::Unpause()
    : AbstractBuiltin( "unpause" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Unpause the game.";
}

///////////////////////////////////////////////////////////////////////////////

Unpause::~Unpause()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unpause::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    if (!level.match_pause) {
        txt._ebuf << "Game is not paused.";
        return PA_ERROR;
    }

    G_refPause_cmd( txt._client ? &txt._client->gentity : NULL, qfalse );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
