
#ifndef __WORLD_H
#define __WORLD_H


#include "Common.h"
#include "Timer.h"


#include <atomic>
#include <map>
#include <set>
#include <list>

class WorldSession;
class WorldSocket;

enum WorldIntConfigs
{
	CONFIG_PORT_WORLD = 0,
	CONFIG_SOCKET_TIMEOUTTIME,
	CONFIG_INTERVAL_ROOMUPDATE,
	CONFIG_NUMTHREADS,
	CONFIG_NUMBERROOMS,
	CONFIG_BASICSCORE,
	CONFIG_WAIT_TIME,
	CONFIG_AI_PLAYER_COUNT,
	CONFIG_ROOM1_GOLD,
	CONFIG_ROOM2_GOLD,
	CONFIG_ROOM3_GOLD,
	CONFIG_ROOM4_GOLD,
	CONFIG_ROOM5_GOLD,
	CONFIG_ROOM6_GOLD,
	INT_CONFIG_VALUE_COUNT
};

enum ShutdownExitCode
{
	SHUTDOWN_EXIT_CODE = 0,
	ERROR_EXIT_CODE = 1,
	RESTART_EXIT_CODE = 2
};

typedef std::unordered_map<uint32, WorldSession*> SessionMap;


/// The World
class World
{
public:
	static World* instance()
	{
		static World instance;
		return &instance;
	}

	static std::atomic<uint32> m_worldLoopCounter;

	void AddSession(WorldSession* s);
	bool RemoveSession(uint32 id);

	void SetInitialWorldSettings();
	void LoadConfigSettings(bool reload = false);

	static void StopNow(uint8 exitcode) { m_stopEvent = true; m_ExitCode = exitcode; }
	static bool IsStopped() { return m_stopEvent; }

	void Update(uint32 diff);

	void UpdateSessions(uint32 diff);

	/// Get a server configuration element (see #WorldConfigs)
	uint32 getIntConfig(WorldIntConfigs index) const
	{
		return index < INT_CONFIG_VALUE_COUNT ? m_int_configs[index] : 0;
	}

private:
	World();
	~World();

	static std::atomic<bool> m_stopEvent;
	static uint8 m_ExitCode;
	SessionMap m_sessions;

	// sessions that are added async
	void AddSession_(WorldSession* s);
	LockedQueue<WorldSession*> addSessQueue;

	uint32 m_int_configs[INT_CONFIG_VALUE_COUNT];
};

#define sWorld World::instance()

#endif