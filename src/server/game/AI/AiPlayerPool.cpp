#include "AiPlayerPool.h"

#include<stdio.h>
#include "Player.h"
#include "World.h"

AiPlayerPool::AiPlayerPool()
{
	uint32 defaultAiCount = sWorld->getIntConfig(CONFIG_AI_PLAYER_COUNT);

	for (uint32 i = 0; i < defaultAiCount; ++i) 
	{
		Player* player = new Player(nullptr);
		_aiPlayerList.push_back(player);
	}
}

AiPlayerPool::~AiPlayerPool()
{
	for (Player* player : _aiPlayerList)
	{
		delete player;
	}
}

Player * AiPlayerPool::getAiPlayer(uint32 roomid)
{
	std::lock_guard<std::recursive_mutex> lockClass(_aiPlayerPoolLock);

	if (_aiPlayerList.empty()) 
	{
		Player* player = new Player(nullptr);
		_aiPlayerList.push_back(player);
	}

	Player *player = _aiPlayerList.back();
		
	_aiPlayerList.pop_back();
	
	configureAiPlayer(player, roomid);
	printf("getAiPlayer: %d,now ai count: %d\n", player->getid(),_aiPlayerList.size());
	return player;
}

void AiPlayerPool::configureAiPlayer(Player * player, uint32 roomid)
{
	PlayerInfo aiPlayerInfo;
	
	aiPlayerInfo.id = 2000 + rand() % 100;
	aiPlayerInfo.sex = rand() % 2;
	aiPlayerInfo.gold = sWorld->getIntConfig(WorldIntConfigs(CONFIG_ROOM1_GOLD + roomid));
	aiPlayerInfo.level = roomid * 2 + 1;
	aiPlayerInfo.all_Chess = roomid * 100 + 80;
	aiPlayerInfo.win_chess = aiPlayerInfo.all_Chess * 0.4;
	aiPlayerInfo.win_Rate = 0.4;
	snprintf(aiPlayerInfo.account, 12,"%d", aiPlayerInfo.id + 524288);
	memcpy(aiPlayerInfo.nick_name, "天天向上",8);

	player->initPlayer();

	player->loadData(aiPlayerInfo);
	player->setPlayerType(PLAYER_TYPE_AI);
	player->setStart();


}

void AiPlayerPool::releasePlayer(Player * player)
{
	_aiPlayerPoolLock.lock();
	_aiPlayerList.push_back(player);
	static uint32  releaseAiCount = 0;
	printf("releasePlayer,now ai count: %d\n", _aiPlayerList.size());
	_aiPlayerPoolLock.unlock();
}