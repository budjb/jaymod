#ifndef GAME_USER_H
#define GAME_USER_H

///////////////////////////////////////////////////////////////////////////////

class UserDB;

///////////////////////////////////////////////////////////////////////////////

/*
 * User class represents a persistent data record for every unique
 * GUID-based player.
 *
 * For all practical purposes we assume GUID key is guaranteed to be unique
 * and the success of this implementation hinges on that assumption.
 *
 */
class User {
    friend class UserDB;

private:
    string _guid;
    
public:
    User ( );
    User ( bool );
    User ( const User& );

    ~User ();

    bool  operator== ( const User& ) const;
    bool  operator!= ( const User& ) const;
    User& operator=  ( const User& );

    void canonicalizePrivileges();

    bool hasPrivilege ( const Privilege& ) const;
    void reset        ( );
    void xpReset      ( );

    /**************************************************************************
     * Returns exactly: this == &BAD .
     */
    bool isNull() const;

    /**************************************************************************
     * Main attributes.
     */
    const string&  guid;           // globally unique ID assigned by punkbuster
    bool           fakeguid;       // track locally assigned guids
    time_t         timestamp;      // time of last connect/disconnect
    string         ip;             // client IPv4 address updated on connect
    string         mac;            // client MAC address updated on connect
    string         name;           // client name updated on connect and change
    string         namex;          // name w/ extended-color-codes
    string         greetingText;   // broadcast greeting on connect
    string         greetingAudio;  // audio to play on connect
    int            authLevel;      // authorization level

    PrivilegeSet* privGranted;  // per-user granted
    PrivilegeSet* privDenied;   // per-user denied

    /**************************************************************************
     * XP-save attributes.
     */
    float  xpSkills[ SK_NUM_SKILLS ];  // XP for each skill

    /**************************************************************************
     * Mute attributes.
     *
     */
    bool   muted;           // true when muted, when false mute* fields are ignored
    time_t muteTime;        // time of mute
    time_t muteExpiry;      // when mute expires
    string muteReason;      // reason for mute
    string muteAuthority;   // name of user who muted this user
    string muteAuthorityx;  // muteAuthority w/ extended-color-codes

    /**************************************************************************
     * Ban attributes.
     */
    bool   banned;         // true when banned, when false ban* fields are ignored
    time_t banTime;        // time of ban
    time_t banExpiry;      // time when ban expires, 0 == permanent
    string banReason;      // reason for ban
    string banAuthority;   // name of user who banned this user
    string banAuthorityx;  // banAuthority w/ extended-color-codes

    /**************************************************************************
     * Note attributes.
     */
    vector<string> notes;

private:
    void  decode ( map<string,string>& );
    void  encode ( ostream&, int );

public:
    static User BAD;
    static User DEFAULT;
    static User CONSOLE;

    static const vector<string>::size_type notesMax;

private:
    static void scramble( char*, int );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_USER_H
