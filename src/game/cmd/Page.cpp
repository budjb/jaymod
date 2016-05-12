#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Page::Page()
    : AbstractBuiltin( "page", true )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "NUM" );
    __descr << "Show a page# from the last command which produced long output.";
}

///////////////////////////////////////////////////////////////////////////////

Page::~Page()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Page::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    Report& report = (txt._client ? clientReports[txt._client->slot] : consoleReport);

    if (txt._args.size() == 1) {
        Buffer buf;
        buf << _name << ":"
            << " You have " << xvalue( report.size() )
            << " page" << (report.size() == 1 ? "" : "s")
            << " available.";
        printChat( txt._client, buf );
        return PA_NONE;
    }

    // bail on empty report
    if (report.empty()) {
        txt._ebuf << "No pages available.";
        return PA_ERROR;
    }

    int num = atoi( txt._args[1].c_str() );
    if (num < 1)
        num = 1;
    else if ((Report::size_type)num > report.size())
        num = report.size();

    print( txt._client, *report[num-1] );

    Buffer b( xcheader );
    b << "--page " << xheaderBOLD( num ) << " of " << xheaderBOLD( report.size() );
    print( txt._client, b );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

void
Page::report( Client* client, const Buffer& buffer )
{
    if (buffer.numLines <= maxLines) {
        print( client, buffer );
        return;
    }

    Report& report = client ? clientReports[client->slot] : consoleReport;
    buffer.toReport( report, maxLines, maxPages );
    print( client, *report[0] );

    Buffer b( xcheader );
    b << "--page " << xheaderBOLD( "1" ) << " of " << xheaderBOLD( report.size() );
    print( client, b );
}

///////////////////////////////////////////////////////////////////////////////

Page::Report::Report()
{
}

///////////////////////////////////////////////////////////////////////////////

Page::Report::~Report()
{
    const size_type max = size();
    for ( size_type i = 0; i < max; i++ )
        delete operator[]( i );
}

///////////////////////////////////////////////////////////////////////////////

void
Page::Report::clear()
{
    const size_type max = size();
    for ( size_type i = 0; i < max; i++ )
        delete operator[]( i );
    vector<Buffer*>::clear();
}

///////////////////////////////////////////////////////////////////////////////

const uint32 Page::maxLines = 20;
const uint32 Page::maxPages = 99;

///////////////////////////////////////////////////////////////////////////////

Page::Report Page::consoleReport;
Page::Report Page::clientReports[ MAX_CLIENTS ];

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
