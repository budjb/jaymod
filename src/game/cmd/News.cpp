#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

News::News()
    : AbstractBuiltin( "news" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "MAPNAME" );
    __descr << "Plays the 'news' for the current or specified map.";
}

///////////////////////////////////////////////////////////////////////////////

News::~News()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
News::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    string mapname = level.rawmapname;
    if (txt._args.size() == 2)
        mapname = txt._args[1];

    ostringstream oss;
    oss << "sound/vo/" << mapname << "/news_" << mapname << ".wav";
    const string soundfile = oss.str();

        G_globalSound( soundfile.c_str() );

    Buffer buf;
    buf << _name << ": " << xvalue( soundfile );
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
