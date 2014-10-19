
#ifndef __WORLD_H
#define __WORLD_H


#include "Common.h"
#include "Timer.h"

enum WorldIntConfigs
{
	CONFIG_PORT_WORLD = 0,
	INT_CONFIG_VALUE_COUNT,
	CONFIG_SOCKET_TIMEOUTTIME
};

/// The World
class World
{
public:
	static World* instance()
	{
		static World instance;
		return &instance;
	}

	void SetInitialWorldSettings();
	void LoadConfigSettings(bool reload = false);

	/// Get a server configuration element (see #WorldConfigs)
	uint32 getIntConfig(WorldIntConfigs index) const
	{
		return index < INT_CONFIG_VALUE_COUNT ? m_int_configs[index] : 0;
	}

private:
	World();
	~World();

	uint32 m_int_configs[INT_CONFIG_VALUE_COUNT];
};

#define sWorld World::instance()

#endif