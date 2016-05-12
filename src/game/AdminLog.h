#ifndef GAME_ADMINLOG_H
#define GAME_ADMINLOG_H

///////////////////////////////////////////////////////////////////////////////

class AdminLog
{
private:

    string   _filename;
    ofstream _out;

public:
    AdminLog();
    ~AdminLog();

    void init();
    void log( Client*, const vector<string>&, bool );
    void recompute();

public:
    static void cvarCallback( Cvar& );
};

///////////////////////////////////////////////////////////////////////////////

extern AdminLog adminLog;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ADMINLOG_H
