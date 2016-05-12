#ifndef GAME_MAPRECORD_H
#define GAME_MAPRECORD_H

///////////////////////////////////////////////////////////////////////////////

class MapDB;

///////////////////////////////////////////////////////////////////////////////

class MapRecord {
    friend class MapDB;

private:
    string _name;
    
public:
    MapRecord  ( );
    MapRecord  ( const MapRecord& mapRecord );
    ~MapRecord ( );

    /**************************************************************************
     * Main attributes.
     *
     */
    const string&  name;
    time_t         timestamp;
    long           count;

    // Longest killing spree
    int            longestSpree;
    string         longestSpreeName;
    string         longestSpreeNamex;
    time_t         longestSpreeTime;

    MapRecord&     operator=( const MapRecord& mapRecord );
    bool           operator==( const MapRecord& mapRecord ) const;

private:
    void  decode ( map<string,string>& );
    void  encode ( ostream&, int );
    bool  isNull ( );

public:
    static MapRecord BAD;
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_MAPRECORD_H
