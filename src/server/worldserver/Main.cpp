#include <boost/asio.hpp>
#include <thread>

#include "Configuration/Config.h"

#ifndef _LANDLORD_CORE_CONFIG
#define _LANDLORD_CORE_CONFIG  "roomServer.conf"
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
		

	return 0;
}

void SignalHandler(const boost::system::error_code& error, int /*signalNumber*/)
{
	//if (!error)
		//World::StopNow(SHUTDOWN_EXIT_CODE);
}