#pragma once

namespace love
{
	class Timer
	{
		static love::Timer * instance;

		public:
			static love::Timer * Instance()
			{
				if (!instance)
					instance = new love::Timer();
				return instance;
			}

			static int GetTime(lua_State * L);
			static int GetDelta(lua_State * L);
			static int GetFPS(lua_State * L);
			static int Step(lua_State * L);

		private:
			Timer();

			static float dt;
			static float lastTick;
			static float currentTick;
	};
}