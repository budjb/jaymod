#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Pause::Pause()
    : AbstractBuiltin( "pause" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Pause the game.";
}

///////////////////////////////////////////////////////////////////////////////

Pause::~Pause()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Pause::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    if (level.match_pause) {
        txt._ebuf << "Game is already paused.";
        return PA_ERROR;
    }

    G_refPause_cmd( txt._client ? &txt._client->gentity : NULL, qtrue );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
