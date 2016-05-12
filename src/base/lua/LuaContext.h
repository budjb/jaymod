#ifndef __LUACONTEXT_H
#define __LUACONTEXT_H

#include <base/public.h>
#include <base/lua/public.h>

class LuaContext
{
private:
	lua_State* state;
	string errorMsg;
public:
	LuaContext(string code);

private:
	void Log(const char* format, ...);
};

#endif // LUACONTEXT_H
