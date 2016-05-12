#include <base/public.h>

namespace lua {

LuaContext::LuaContext(string code)
{
	state = luaL_newstate();
}
/*
void LuaContext::Log(const char* format, ...)
{
	va_list args;
	char buffer[1024];

	va_start(args, format);
	Q_vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
}
*/

}

