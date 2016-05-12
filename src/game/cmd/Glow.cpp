#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Glow::Glow()
    : AbstractBuiltin( "glow" )
{
    __usage << xvalue( "!" + _name )
            << " [" << xvalue( "on" ) << '|' << xvalue( "off" ) << ']';

    __descr << "All players emit a colored glow. No argument reports status.";
}

///////////////////////////////////////////////////////////////////////////////

Glow::~Glow()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Glow::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    const bool enabled = cvars::bg_glow.ivalue;
    Buffer buf;

    // If there are no arguments, just report status
    if (txt._args.size() == 1) {
        buf << _name << ": " << "Glow is " << xvalue( enabled ? "enabled" : "disabled" ) << '.';
        printChat( txt._client, buf );
        return PA_NONE;
    }
        
    string action = txt._args[1];
    str::toLower(action);
    
    if (action == "on") {
        if (enabled) {
            txt._ebuf << "Glow is already " << xvalue( "enabled" ) << '.';
            return PA_ERROR;
        }

        cvars::bg_glow.set( "1" );
        buf << _name << ": Glow is now " << xvalue( "enabled" ) << '.';
        printCpm( txt._client, buf, true );
    }
    else if (action == "off") {
        if (!enabled) {
            txt._ebuf << "Glow is already " << xvalue( "disabled" ) << '.';
            return PA_ERROR;
        }
        
        cvars::bg_glow.set( "0" );
        buf << _name << ": Glow is now " << xvalue( "disabled" ) << '.';
        printCpm( txt._client, buf, true );
    }
    else {
        txt._ebuf << "Invalid argument: " << xvalue( txt._args[1] );
        return PA_ERROR;
    }

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
