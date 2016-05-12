#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

About::About()
    : AbstractBuiltin( "about", true )
{
    __usage << xvalue( "!" + _name );
    __descr << "Shows information about the version of " << xnone( JAYMOD_namex ) << " that is currently installed.";
}

///////////////////////////////////////////////////////////////////////////////

About::~About()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
About::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    InlineText colA;
    InlineText colB = xvalue;

    colA.flags  |= ios::left;
    colA.width   = 14;
    colA.suffix  = ":";

    Buffer buf;
    buf << xnone(JAYMOD_titlex)
        << '\n' << colA( "URL homepage" ) << colB( JAYMOD_website )
        << '\n' << colA( "IRC channel" )  << colB( JAYMOD_irc )
        << '\n' << colA( "build date" )   << colB( JAYMOD_buildDate );

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
