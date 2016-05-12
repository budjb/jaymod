#include <bgame/impl.h>

Location::Location( float x, float y, float z, const string& desc )
    : description ( desc )
{
    origin[0] = x;
    origin[1] = y;
    origin[2] = z;
}

Location::~Location()
{
}
