#include <boost/asio.hpp>
#include <thread>

#include "AsyncAcceptor.h"
#include "Configuration/Config.h"
#include "Log.h"
#include "World.h"
#include "WorldSocket.h"


#ifndef _LANDLORD_CORE_CONFIG
#define _LANDLORD_CORE_CONFIG  "worldserver.conf"
#endif

boost::asio::io_service _ioService;

void SignalHandler(const boost::system::error_code& error, int signalNumber);

int main(int argc, char* argv[])
{
	std::string configFile = _LANDLORD_CORE_CONFIG;

	std::string configError;
	if (!sConfigMgr->LoadInitial(configFile, configError))
	{
		printf("Error in config file: %s\n", configError.c_str());
		return 1;
	}

	TC_LOG_TRACE("network.opcode", "C->S: ");

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
	//sWorld->SetInitialWorldSettings();

	// Launch the worldserver listener socket
//	uint16 worldPort = uint16(sWorld->getIntConfig(CONFIG_PORT_WORLD));
//	std::string worldListener = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");
//	bool tcpNoDelay = sConfigMgr->GetBoolDefault("Network.TcpNodelay", true);

//	AsyncAcceptor<WorldSocket> worldAcceptor(_ioService, worldListener, worldPort, tcpNoDelay);

	getchar();
	return 0;
}

void SignalHandler(const boost::system::error_code& error, int /*signalNumber*/)
{
	//if (!error)
		//World::StopNow(SHUTDOWN_EXIT_CODE);
}