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

	void AddPlayer(uint32 id,Player *player);
private:
	void UpdateOne(uint32 diff);
	void UpdateTwo(uint32 diff);

	typedef std::unordered_map<uint32, Player*> PlayerMapType;	
	typedef std::list<Player *> onePlayerList;
	typedef std::pair<Player*, Player*> twoPlayer;
	typedef std::list<twoPlayer> twoPlayerList;
	typedef std::pair<Player*, twoPlayer> threePlayer;
	typedef std::list<threePlayer> threePlayerList;

	PlayerMapType _playerMap;
	onePlayerList _OnePlayerList;
	twoPlayerList  _twoPlayerList;
	threePlayerList _threePlayerList;
	
	uint32 _id;
	uint32 _basic_score;
};

#endif