#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

///////////////////////////////////////////////////////////////////////////////

class Level {
    friend class LevelDB;

private:
    int _level;

public:
    Level ( );
    Level ( const Level& );

    ~Level();

    bool   operator== ( const Level& ) const;
    bool   operator!= ( const Level& ) const;
    Level& operator=  ( const Level& );

    /**************************************************************************
     * Returns exact same as: this == &BAD .
     */
    bool isNull() const;

    /**************************************************************************
     * Main attributes.
     *
     */
    const int& level;
    string     name;
    string     namex;
    string     greetingText;
    string     greetingAudio;

    PrivilegeSet privGranted;  // granted privileges
    PrivilegeSet privDenied;   // denied privileges

private:
    void decode( map<string,string>& data );  // decodes record from database
    void encode( ostream&, int );  // encodes record to database

public:
    static Level BAD;
    static Level DEFAULT;

    static const int NUM_MIN;
    static const int NUM_MAX;
    static const int CUSTOM_MIN;
    static const int CUSTOM_MAX;
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_LEVEL_H
