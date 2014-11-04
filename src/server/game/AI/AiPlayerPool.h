#ifndef __AI_PLAYER_POOL_H
#define __AI_PLAYER_POOL_H

class Player;

class AiPlayerPool
{
public:
	static AiPlayerPool* instance()
	{
		static AiPlayerPool instance;
		return &instance;
	}

	Player * getAiPlayer(uint32 roomid);
	void releasePlayer(Player * player);

private:
	typedef std::list<Player *> aiPlayerList;

	aiPlayerList _aiPlayerList;
	std::recursive_mutex _aiPlayerPoolLock;

	void configureAiPlayer(Player * player, uint32 roomid);

	AiPlayerPool();
	~AiPlayerPool();
};

#define sAiPlayerPool AiPlayerPool::instance()

#endif