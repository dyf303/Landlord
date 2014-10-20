#include "World.h"
#include "Configuration/Config.h"
#include "WorldSession.h"


std::atomic<bool> World::m_stopEvent(false);
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
std::atomic<uint32> World::m_worldLoopCounter(0);


World::World()
{

}

World::~World()
{

}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
	///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
	SessionMap::const_iterator itr = m_sessions.find(id);

	if (itr != m_sessions.end() && itr->second)
	{
		//if (itr->second->PlayerLoading())
		//	return false;

		itr->second->KickPlayer();
	}

	return true;
}

void World::AddSession(WorldSession* s)
{
	addSessQueue.add(s);
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

/// Update the World !
void World::Update(uint32 diff)
{

}

void World::UpdateSessions(uint32 diff)
{
	///- Add new sessions
	WorldSession* sess = NULL;
	while (addSessQueue.next(sess))
		AddSession_(sess);

	///- Then send an update signal to remaining ones
	for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
	{
		next = itr;
		++next;

		///- and remove not active sessions from the list
		WorldSession* pSession = itr->second;
		WorldSessionFilter updater(pSession);

		if (!pSession->Update(diff, updater))    // As interval = 0
		{
			if (!RemoveQueuedPlayer(itr->second) && itr->second && getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
				m_disconnects[itr->second->GetAccountId()] = time(NULL);
			RemoveQueuedPlayer(pSession);
			m_sessions.erase(itr);
			delete pSession;

		}
	}
}

/// Kick (and save) all players
void World::KickAll()
{
	//m_QueuedPlayer.clear();                                 // prevent send queue update packet and login queued sessions

	// session not removed at kick and will removed in next update tick
	for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
		itr->second->KickPlayer();
}