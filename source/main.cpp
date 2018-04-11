extern "C" 
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>

	#include <compat-5.2.h>
	#include <luaobj.h>
}

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SDL.h>

#include "modules/audio.h"
#include "common/exception.h"
#include "modules/filesystem.h"
#include "modules/graphics.h"
#include "modules/window.h"
#include "modules/system.h"

#include "modules/timer.h"

#include <stdio.h>
#include <stdarg.h>
#include <map>
#include <vector>

#include <switch.h>

#include "objects/gamepad/gamepad.h"
#include "objects/gamepad/wrap_gamepad.h"

#include "socket/socket.h"
#include "modules/love.h"
#include "modules/joystick.h"

#include "boot_lua.h"

#include "common/types.h"
#include "common/util.h"

bool ERROR = false;
bool LOVE_QUIT = false;

int main()
{
	//Console::Initialize();

	System::Initialize();

	Graphics::Initialize();
	
	Window::Initialize();

	Audio::Initialize();

	Filesystem::Initialize();

	lua_State * L = luaL_newstate();

	luaL_openlibs(L);

	love_preload(L, Socket::Initialize, "socket");

	luaL_requiref(L, "love", Love::Initialize, 1);

	Joystick::Initialize(L);

	lua_catchexception(L, [L]() 
	{
		if (luaL_dobuffer(L, (char *)boot_lua, boot_lua_size, "boot"))
			throw Exception(L);
	});

	/*
	** aptMainLoop important code moved to love.cpp
	** this was to register it as love.run
	** for error handling purposes
	*/
	
	while (appletMainLoop())
	{
		if (Love::IsRunning())
			luaL_dostring(L, "xpcall(love.run, love.errhand)");
		else
			break;
	}

	Socket::Close();

	Love::Exit(L);

	return 0;
}
