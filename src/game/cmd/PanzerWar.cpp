#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

PanzerWar::PanzerWar()
    : AbstractBuiltin( "panzerwar" )
{
    __usage << xvalue( "!" + _name ) << " [" << xvalue( "on" ) << '|' << xvalue( "off" ) << ']';
    __descr << "A panzerfaust frag-fest. No argument reports status.";
}

///////////////////////////////////////////////////////////////////////////////

PanzerWar::~PanzerWar()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
PanzerWar::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    const bool enabled = cvars::bg_panzerWar.ivalue;
    Buffer buf;

    // If there are no arguments, just report status
    if (txt._args.size() == 1) {
        buf << _name << ": Panzer war is " << xvalue( enabled ? "enabled" : "disabled" ) << '.';
        printChat( txt._client, buf );
        return PA_NONE;
    }
        
    string action = txt._args[1];
    str::toLower(action);
    
    if (action == "on") {
        if (enabled) {
            txt._ebuf << "Panzer war is already " << xvalue( "enabled" ) << '.';
            return PA_ERROR;
        }

		for (int i = 0; i < level.numConnectedClients; i++) {
            gentity_t* ent = &g_entities[i];

            if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
                continue;

            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, DAMAGE_JAY_NO_PROTECTION, MOD_UNKNOWN );
		}

        cvars::bg_panzerWar.set( "1" );
        buf << _name << ": Panzer war is now " << xvalue( "enabled" ) << '.';
        printCpm( txt._client, buf, true );
    }
    else if (action == "off") {
        if (!enabled) {
            txt._ebuf << "Panzer war is already " << xvalue( "disabled" ) << '.';
            return PA_ERROR;
        }

		for (int i = 0; i < level.numConnectedClients; i++) {
            gentity_t* ent = &g_entities[i];

            if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
                continue;

            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, DAMAGE_JAY_NO_PROTECTION, MOD_UNKNOWN );
		}

        cvars::bg_panzerWar.set( "0" );
        buf << _name << ": Panzer war is now " << xvalue( "disabled" ) << '.';
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
