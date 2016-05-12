#ifndef BGAME_PROCESS_H
#define BGAME_PROCESS_H

///////////////////////////////////////////////////////////////////////////////

class Process {
public:
    typedef unsigned long long mstime_t;  // milliseconds time value

private:
    bool _pendingReload;
    bool _pendingShutdown;

public:
    Process();
    ~Process();

    void     init     ();  // called early during game-init
    void     shutdown ();  // called late during game-shutdown
    mstime_t mstime   ();  // get milliseconds since epoch

    void beginCriticalSection ();  // put this around code which must not get interrupted
    void endCriticalSection   ();  // put this around code which must not get interrupted

    void setPendingReload   ( bool );
    void setPendingShutdown ( bool );

    const bool& pendingReload;   // SDK checks this to see if SIGUSR1 signal was caught
    const bool& pendingShutdown; // SDK checks this to see if HUP,TERM signal was caught

private:
    void signalInit     ();
    void signalShutdown ();
};

///////////////////////////////////////////////////////////////////////////////

extern Process process;

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_PROCESS_H
