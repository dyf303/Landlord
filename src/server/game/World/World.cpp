#include "World.h"
#include "Configuration/Config.h"

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

/// Initialize config values
void World::LoadConfigSettings(bool reload)
{
	if (reload)
	{
		std::string configError;
		if (!sConfigMgr->Reload(configError))
		{
			TC_LOG_ERROR("misc", "World settings reload fail: %s.", configError.c_str());
			return;
		}
		sLog->LoadFromConfig();
	}
	if (reload)
	{
		uint32 val = sConfigMgr->GetIntDefault("WorldServerPort", 8085);
		if (val != m_int_configs[CONFIG_PORT_WORLD])
			TC_LOG_ERROR("server.loading", "WorldServerPort option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_PORT_WORLD]);
	}
	else
		m_int_configs[CONFIG_PORT_WORLD] = sConfigMgr->GetIntDefault("WorldServerPort", 8085);
}