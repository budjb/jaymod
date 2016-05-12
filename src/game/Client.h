#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

///////////////////////////////////////////////////////////////////////////////

class Client
{
private:
    int  calculateKnockback ( int, vec3_t );
    bool headshotAllowed    ( const int );

    int  _needGreeting;
    int  _numNameChanges;

public:
    Client();
    ~Client();

    void recordHit( AbstractHitVolume::zone_t, bool );
    void addStats ( AbstractHitVolume::zone_t, int  );

    void takeBulletDamageFrom( const TraceContext&, Client&, int, bool, const int mod );

    void init            ( );  // invoked from game init
    bool isReconcileSafe ( );  // is it safe to backwards reconcile?
    bool isVisibleTarget ( );  // is the player visible and a potential target?
    void shutdown        ( );  // invoked from game shutdown
    void reset           ( );
    void run             ( );  // invoked after  ClientEndFrame
    void think           ( );  // invoked before ClientThink
    void xprint          ( const text::Buffer& );

    void xpBackup  ( );  // backup XP state to DB
    void xpReset   ( );  // reset XP state
    void xpRestore ( );  // restore XP state from DB

    void notifyConnecting( bool );
    void notifyBegin();

    void notifyNameChanged();

    void greeting(); // do something when a player joins

    const int  slot;
    ostream    debug;
    ostream    print;
    gclient_t& gclient;
    gentity_t& gentity;

    AbstractBulletModel* bulletModel;
    AbstractHitModel*    hitModel;

    const int& numNameChanges;

    // Antiwarp
    list<usercmd_t>  cmdQueue;
    int              cmdCount;
    float            cmdDelta;
    int              cmdLastRealTime;
};

///////////////////////////////////////////////////////////////////////////////

extern Client g_clientObjects[MAX_CLIENTS];

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CLIENT_H
