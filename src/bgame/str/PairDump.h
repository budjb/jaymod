#ifndef BGAME_STR_PAIRDUMP_H
#define BGAME_STR_PAIRDUMP_H

///////////////////////////////////////////////////////////////////////////////

class PairDump
{
private:
    struct Data {
        string name;
        string value;
    };

    struct Section {
        string     name;
        list<Data> entries;
    };

private:
    const string  _header;
    list<Section> _sections;
    Section*      _section;

public:
    PairDump( string );
    ~PairDump();

    void addEntry ( string, bool );
    void addEntry ( string, int32 );
    void addEntry ( string, uint32 );
    void addEntry ( string, float );
    void addEntry ( string, double );
    void addEntry ( string, string );
    void addEntry ( string, const vec3_t& );

    void addSection( string );

    void clear();

    void dump( ostream& );
    void dump( const PairDump&, ostream& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_STR_PAIRDUMP
