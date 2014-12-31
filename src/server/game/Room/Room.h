#ifndef _ROOM_H
#define _ROOM_H

#include "Timer.h"

class Player;

class Room
{
public:
	Room(uint32 id, uint32 basic_score);
	~Room();
	uint32 getRoomId(){ return _id; };
	void Update(const uint32 diff);

	void AddPlayer(uint32 id,Player *player,bool inOne = true);
	Player * findPlayer(uint32 id);

	typedef std::unordered_map<uint32, Player*> PlayerMapType;
	typedef std::list<Player *> onePlayerList;
	typedef std::pair<Player*, Player*> twoPlayer;
	typedef std::list<twoPlayer> twoPlayerList;
	typedef std::pair<twoPlayer, Player*> threePlayer;
	typedef std::list<threePlayer> threePlayerList;
private:
	void UpdateOne(uint32 diff);
	Player * getPlayerFromOne();

	void UpdateTwo(uint32 diff);
	bool  LogoutTwo(twoPlayer &twoP);

	void UpdateThree(uint32 diff);
	bool LogoutThree(threePlayer &threeP);
	bool allStart(threePlayer &threeP);
	bool allAtThree(threePlayer &threeP);
	bool allWaitDealCards(threePlayer &threeP);
	bool roundOver(threePlayer &threeP);
	void releaseAiPlayer(threePlayer &threeP);

	void dealCards(threePlayer &threeP);
	void shuffleCard(uint8* Cards);


	PlayerMapType _playerMap;
	onePlayerList _OnePlayerList;
	twoPlayerList  _twoPlayerList;
	threePlayerList _threePlayerList;
	
	uint32 _id;
	uint32 _basic_score;
};

#endif