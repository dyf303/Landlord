#include <boost/asio.hpp>
#include <thread>

#include "AsyncAcceptor.h"
#include "Configuration/Config.h"
#include "Log.h"
#include "RoomManager.h"
#include "World.h"
#include "WorldSocket.h"


#ifndef _LANDLORD_CORE_CONFIG
#define _LANDLORD_CORE_CONFIG  "worldserver.conf"
#endif

#define WORLD_SLEEP_CONST 50

boost::asio::io_service _ioService;

void SignalHandler(const boost::system::error_code& error, int signalNumber);

void WorldUpdateLoop();

void ShutdownThreadPool(std::vector<std::thread>& threadPool);

int main(int argc, char* argv[])
{
	
	std::string configFile = _LANDLORD_CORE_CONFIG;
	std::string configError;
	if (!sConfigMgr->LoadInitial(configFile, configError))
	{
		printf("Error in config file: %s\n", configError.c_str());
		return 1;
	}
	boost::asio::signal_set signals(_ioService, SIGINT, SIGTERM);
#if PLATFORM == PLATFORM_WINDOWS
	signals.add(SIGBREAK);
#endif
	signals.async_wait(SignalHandler);

	int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
	std::vector<std::thread> threadPool;

	if (numThreads < 1)
		numThreads = 1;

	for (int i = 0; i < numThreads; ++i)
		threadPool.push_back(std::thread(boost::bind(&boost::asio::io_service::run, &_ioService)));

	// Initialize the World
	sWorld->SetInitialWorldSettings();
	// Launch the worldserver listener socket
	uint16 worldPort = uint16(sWorld->getIntConfig(CONFIG_PORT_WORLD));
	std::string worldListener = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");
	bool tcpNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

	AsyncAcceptor<WorldSocket> worldAcceptor(_ioService, worldListener, worldPort, tcpNoDelay);

	WorldUpdateLoop();

	// Shutdown starts here
	ShutdownThreadPool(threadPool);

	sRoomMgr->UnloadAll();

	return 0;
}

void SignalHandler(const boost::system::error_code& error, int /*signalNumber*/)
{
	if (!error)
		World::StopNow(SHUTDOWN_EXIT_CODE);
}

void WorldUpdateLoop()
{
	uint32 realCurrTime = 0;
	uint32 realPrevTime = getMSTime();

	uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

	///- While we have not World::m_stopEvent, update the world
	while (!World::IsStopped())
	{
		++World::m_worldLoopCounter;
		realCurrTime = getMSTime();

		uint32 diff = getMSTimeDiff(realPrevTime, realCurrTime);

		sWorld->Update(diff);
		realPrevTime = realCurrTime;

		// diff (D0) include time of previous sleep (d0) + tick time (t0)
		// we want that next d1 + t1 == WORLD_SLEEP_CONST
		// we can't know next t1 and then can use (t0 + d1) == WORLD_SLEEP_CONST requirement
		// d1 = WORLD_SLEEP_CONST - t0 = WORLD_SLEEP_CONST - (D0 - d0) = WORLD_SLEEP_CONST + d0 - D0
		if (diff <= WORLD_SLEEP_CONST + prevSleepTime)
		{
			prevSleepTime = WORLD_SLEEP_CONST + prevSleepTime - diff;
		
			std::this_thread::sleep_for(std::chrono::milliseconds(prevSleepTime));
		}
		else
			prevSleepTime = 0;
	}
}

void ShutdownThreadPool(std::vector<std::thread>& threadPool)
{
	_ioService.stop();

	for (auto& thread : threadPool)
	{
		thread.join();
	}
}