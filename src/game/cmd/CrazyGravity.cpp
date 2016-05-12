#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

CrazyGravity::CrazyGravity()
    : AbstractBuiltin( "crazygravity" )
{
    __usage << xvalue( "!" + _name ) << " [" << xvalue( "on" ) << '|' << xvalue( "off" ) << ']';
    __descr << "Change to a new random gravity every 30 seconds.  No argument reports status.";
}

///////////////////////////////////////////////////////////////////////////////

CrazyGravity::~CrazyGravity()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
CrazyGravity::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    const bool enabled = level.crazyGravity;
    Buffer buf;

    // If there are no arguments, just report status
    if (txt._args.size() == 1) {
        buf << _name << ": " << "Crazy gravity is " << xvalue( enabled ? "enabled" : "disabled" ) << '.';
        printChat(txt._client, buf);
        return PA_NONE;
    }
        
    string action = txt._args[1];
    str::toLower(action);
    
    if (action == "on") {
        if (enabled) {
            txt._ebuf << "Crazy gravity is already " << xvalue( "enabled" ) << '.';
            return PA_ERROR;
        }

        level.defaultGravity = g_gravity.integer;
        level.crazyGravity = true;
        level.crazyGravityTime = 0;

        buf << _name << ": " << "Crazy gravity is now " << xvalue( "enabled" ) << ".";
        printCpm( txt._client, buf, true );
    }
    else if (action == "off") {
        if (!enabled) {
            txt._ebuf << "Crazy gravity is already " << xvalue( "disabled" ) << '.';
            return PA_ERROR;
        }
        
        trap_Cvar_Set( "g_gravity", va( "%i", level.defaultGravity ));
        level.crazyGravity = false;
        level.crazyGravityTime = 0;

        buf << _name << ": " << "Crazy gravity is now " << xvalue( "disabled" )
            << ". Resetting gravity to " << xvalue( level.defaultGravity ) << '.';
        printCpm( txt._client, buf, true );
    }
    else {
        txt._ebuf << "Invalid argument: " << xvalue( txt._args[1] );
        return PA_ERROR;
    }

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
CrazyGravity::run()
{
    if (!level.crazyGravity)
        return;

    if (level.time - level.crazyGravityTime < 0)
        level.crazyGravityTime = level.time;

    // bail if too early
    if (level.time - level.crazyGravityTime < 25 * 1000)
        return;

    if (level.time - level.crazyGravityTime == 25 * 1000) {
        Buffer buf;
        buf << "crazygravity: Gravity will change in " << xvalue( "5 seconds" );
        printCpm( NULL, buf, true );
        return;
    }

    // bail if too early
    if( level.time - level.crazyGravityTime < 30 * 1000 )
        return;

    // we are changing, reset time
    level.crazyGravityTime = level.time;

    const int newGravValue = rand() % 1200;
    trap_Cvar_Set("g_gravity", va("%i", newGravValue));

    Buffer buf;
    buf << "crazygravity: Gravity has been changed to " << xvalue( newGravValue );
    printCpm( NULL, buf, true );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
