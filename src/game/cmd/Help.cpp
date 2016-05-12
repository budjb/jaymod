#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Help::Help()
    : AbstractBuiltin( "help", true )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "COMMAND" );
    __descr << "Show all Admin System commands you are authorized to use, and gives descriptions.";
}

///////////////////////////////////////////////////////////////////////////////

Help::~Help()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Help::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    if (txt._args.size() == 2) {
        const string& key = txt._args[1];

        AbstractCommand* command = commandForName( key );
        if (!command) {
            txt._ebuf << "Invalid command: " << xvalue( key );
            return PA_ERROR;
        }

        Buffer buf;
        buf << xheader( "-NAME" )
            << '\n' << xlindent << xvalue( command->_name ) << xlunindent
            << '\n' << xheader( "-PRIVILEGE" )
            << '\n' << xlindent << xvalue( command->_privilege._name );

        if (command->_privilege._grantAlways)
            buf << " (" << xbold( "always granted" ) << ')';

        buf << xlunindent
            << '\n' << xheader( "-SYNOPSIS" )
            << '\n' << xlindent << command->_usage << xlunindent
            << '\n' << xheader( "-DESCRIPTION" )
            << '\n' << xlindent << command->_descr << xlunindent;

        print( txt._client, buf );
        return PA_NONE;
    }

    InlineText col;
    col.flags |= ios::left;
    col.width = 13;

    Buffer buf;
    buf << xheader( "-AVAILABLE COMMANDS" );

    int i = 0;
    const Registry::iterator max = registry.end();
    for ( Registry::iterator it = registry.begin(); it != max; it++ ) {
        AbstractCommand& c = *it->second;
        if (!c.hasPermission( txt ))
            continue;

        if (i++ % 5 == 0)
            buf << '\n';

        buf << col( c._name );
    }

    if (i)
        buf << '\n' << xcheader << "--there are " << xheaderBOLD( i ) << " commands available to you.";

    print( txt._client, buf );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
