#ifndef CGAME_LOCATIONDB_H
#define CGAME_LOCATIONDB_H

#include <cgame/cg_local.h>

class LocationDB {
public:
	LocationDB         ( );
	~LocationDB        ( );

	bool load          ( );
    void clean         ( );
    bool getLocation   ( vec3_t, string& );
    bool isLoaded        ( );

private:
    typedef list<Location*> list_t;

	int    line;
    string currentDesc;
    bool   loaded;

	bool open          ( stringstream& );
	int  readInt       ( stringstream&, string& );
    int  readString    ( stringstream&, string& );
    int  readLocation  ( stringstream& );
	int  strip         ( stringstream& );

    list_t locationList;
};

extern LocationDB locationDB;

#endif // CGAME_LOCATIONDB_H
