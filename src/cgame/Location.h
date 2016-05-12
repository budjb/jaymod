#ifndef CGAME_LOCATION_H
#define CGAME_LOCATION_H

#include <cgame/cg_local.h>

class Location {
public:
    Location  ( float, float, float, const string& );
    ~Location ( );

    vec3_t  origin;
    string  description;
};

#endif // CGAME_LOCATION_H
