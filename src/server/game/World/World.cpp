#include "World.h"

#include "Configuration/Config.h"
#include "RoomManager.h"
#include "WorldSession.h"


std::atomic<bool> World::m_stopEvent(false);
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
std::atomic<uint32> World::m_worldLoopCounter(0);


World::World()
{

}

World::~World()
{
	///- Empty the kicked session set
	while (!m_sessions.empty())
	{
		// not remove from queue, prevent loading new sessions
		delete m_sessions.begin()->second;
		m_sessions.erase(m_sessions.begin());
	}
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
	SessionMap::const_iterator itr = m_sessions.find(id);

	if (itr != m_sessions.end() && itr->second)
	{
		return true;
	}

	return false;
}

void World::AddSession(WorldSession* s)
{
	addSessQueue.add(s);
}

void World::AddSession_(WorldSession* s)
{
	ASSERT(s);

	if (RemoveSession(s->getAccountId()))
	{
		s->KickPlayer();
		delete s;                                   
		return;
	}

	m_sessions[s->getAccountId()] = s;
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

	///- Initialize RoomManager
	TC_LOG_INFO("server.loading", "Starting Room System");
	sRoomMgr->Initialize();

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

	m_int_configs[CONFIG_SOCKET_TIMEOUTTIME] = sConfigMgr->GetIntDefault("SocketTimeOutTime", 30000);
	m_int_configs[CONFIG_INTERVAL_ROOMUPDATE] = sConfigMgr->GetIntDefault("RoomUpdateInterval", 100);
	m_int_configs[CONFIG_NUMTHREADS] = sConfigMgr->GetIntDefault("RoomUpdate.Threads", 1);
	m_int_configs[CONFIG_NUMBERROOMS] = sConfigMgr->GetIntDefault("RoomNumbers", 6);
	m_int_configs[CONFIG_BASICSCORE] = sConfigMgr->GetIntDefault("RoomBasicScore", 5000);
	m_int_configs[CONFIG_WAIT_TIME] = sConfigMgr->GetIntDefault("waitTime", 4000);
	m_int_configs[CONFIG_AI_PLAYER_COUNT] = sConfigMgr->GetIntDefault("aiPlayerCount", 10);
	m_int_configs[CONFIG_ROOM1_GOLD] = sConfigMgr->GetIntDefault("room1.Gold", 1000);
	m_int_configs[CONFIG_ROOM2_GOLD] = sConfigMgr->GetIntDefault("room2.Gold", 7000);
	m_int_configs[CONFIG_ROOM3_GOLD] = sConfigMgr->GetIntDefault("room3.Gold", 12000);
	m_int_configs[CONFIG_ROOM4_GOLD] = sConfigMgr->GetIntDefault("room4.Gold", 30000);
	m_int_configs[CONFIG_ROOM5_GOLD] = sConfigMgr->GetIntDefault("room5.Gold", 90000);
	m_int_configs[CONFIG_ROOM6_GOLD] = sConfigMgr->GetIntDefault("room6.Gold", 300000);
	m_int_configs[CONFIG_AI_DELAY] = sConfigMgr->GetIntDefault("aiDelay", 2000);
	

}

/// Update the World !
void World::Update(uint32 diff)
{
	UpdateSessions(diff);
	sRoomMgr->Update(diff);
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

		if (!pSession->Update(diff))    // As interval = 0
		{
			m_sessions.erase(itr);
			delete pSession;
		}
	}
}
