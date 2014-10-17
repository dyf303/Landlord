
#ifndef __WORLD_H
#define __WORLD_H


#include "Common.h"
#include "Timer.h"

/// The World

enum WorldIntConfigs
{
	CONFIG_PORT_WORLD = 0,
	INT_CONFIG_VALUE_COUNT
};
class World
{
public:
	static World* instance()
	{
		static World instance;
		return &instance;
	}

	void SetInitialWorldSettings();

private:
	World();
	~World();

	uint32 m_int_configs[INT_CONFIG_VALUE_COUNT];
};

#define sWorld World::instance()

#endif