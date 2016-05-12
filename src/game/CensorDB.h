#ifndef GAME_CENSORDB_H
#define GAME_CENSORDB_H

///////////////////////////////////////////////////////////////////////////////

class CensorDB {
public:
    typedef set<string> WordSet;

private:
    ifstream  _stream;

    bool   open       ( );
    void   close      ( );

    void   sanitize   ( string&, bool );
    bool   isAlphaNum ( char );
    bool   filterWord ( string& );

    WordSet _wordSet;

public:
    CensorDB    ( );
    ~CensorDB   ( );

    void load   ( );
    bool filter (string&);

    const WordSet& wordSet;
};

///////////////////////////////////////////////////////////////////////////////

extern CensorDB censorDB;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CENSORDB_H
