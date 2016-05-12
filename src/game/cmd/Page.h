#ifndef GAME_CMD_PAGE_H
#define GAME_CMD_PAGE_H

///////////////////////////////////////////////////////////////////////////////

class Page : public AbstractBuiltin
{
protected:
    PostAction doExecute( Context& );

public:
    Page();
    ~Page();

public:
    static void report( Client*, const Buffer& );

    static const uint32 maxLines;
    static const uint32 maxPages;

public:
    class Report : public vector<Buffer*>
    {
    public:
        Report();
        ~Report();

        void clear();
    };

    static Report consoleReport;
    static Report clientReports[ MAX_CLIENTS ];
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_PAGE_H
