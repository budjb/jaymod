#ifndef GAME_USERDB_H
#define GAME_USERDB_H

///////////////////////////////////////////////////////////////////////////////

/*
 * UserDB class represents a persistent database of users keyed by their
 * unique GUIDs.
 *
 * A valid GUID is expected to be 32-characters in length.
 *
 * As clients connect/disconnect the server will maintain a pointer to
 * these users in global table connectedUsers corresponding exactly to
 * client slots.
 *
 * GAME code should NOT store references/pointers to users anywhere except
 * for the global table connectedUsers . All general usage should be
 * short-lived on the vmMain call stack, ie: do not setup global pointers to
 * any User instances returned from UserDB.
 *
 */
class UserDB : public Database {
public:
    typedef map<const string,User>       mapGUID_t;
    typedef multimap<time_t,User*>       mapBANTIME_t;
    typedef multimap<const string,User*> mapMAC_t;
    typedef multimap<const string,User*> mapIP_t; 
    typedef multimap<const string,User*> mapNAME_t;
    typedef multimap<time_t,User*>       mapTIME_t;

    enum BanStatus {
        BAN_NONE,
        BAN_LIFTED,
        BAN_ACTIVE,
    };

private:
    mapGUID_t    _mapGUID;     // primary guid->user memory-map
    mapBANTIME_t _mapBANTIME;  // mac->user index
    mapIP_t      _mapIP;       // ip->user index
    mapMAC_t     _mapMAC;      // mac->user index
    mapNAME_t    _mapNAME;     // name->user index
    mapTIME_t    _mapTIME;     // timestamp->user index

    unsigned int _maxAnonymous;  // max number of users w/ level == 0

public:
    UserDB();
    ~UserDB();

    void load ( bool );  // loads all records from disk
    void save ();        // saves all records to disk

    /*
     * Fetch a user by GUID. If the GUID does not exist, one will be
     * created automatically.
     *
     * Param guid specifies which user GUID to fetch. GUID is case insensitive.
     * Returns a reference to a modifiable user record
     *         or User::NULL if invalid GUID.
     *
     * GAME code should NOT store references/pointers to users anywhere except 
     * for the global table connectedUsers . All general usage should be 
     * short-lived on the vmMain call stack, ie: do not setup global pointers
     * to any User instances returned.
     *
     */
    User&     fetchByKey  ( const string&, string&, bool = false );
    User&     fetchByID   ( const string&, string& );
    bool      fetchByName ( const string&, list<User*>&, string& err );
    void      remove      ( User& );
    BanStatus checkBan    ( string, string, string, User*&, string& );

    void  index   ( User& );  // add to internal indexes
    void  unindex ( User& );  // remove from internal indexes

    uint32 migrateAuth( int, int );  // migrate users from one level to another

    void  purge       ( );  // purge oldest anonymous users if over max
    void  xpResetAll  ( );  // reset all users XP

    const mapGUID_t&    mapGUID;     // primary guid->user memory-map
    const mapBANTIME_t& mapBANTIME;  // banTime   -> user index
    const mapIP_t&      mapIP;       // ip        -> user index
    const mapMAC_t&     mapMAC;      // mac       -> user index
    const mapNAME_t&    mapNAME;     // name      -> user index
    const mapTIME_t&    mapTIME;     // timestamp -> user index

    const unsigned int& maxAnonymous;  // max number of users w/ level == 0
};

///////////////////////////////////////////////////////////////////////////////

extern UserDB userDB;
extern User*  connectedUsers[ MAX_CLIENTS ];

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_USERDB_H
