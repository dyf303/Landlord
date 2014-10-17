#include "World.h"

World::World()
{

}

World::~World()
{

}

/// Initialize the World
void World::SetInitialWorldSettings()
{
	///- Server startup begin
	uint32 startupBegin = getMSTime();

	///- Initialize the random number generator
	srand((unsigned int)time(NULL));

	///- Initialize config settings
	LoadConfigSettings();

	uint32 startupDuration = GetMSTimeDiffToNow(startupBegin);

	TC_LOG_INFO("server.worldserver", "World initialized in %u minutes %u seconds", (startupDuration / 60000), ((startupDuration % 60000) / 1000));
}