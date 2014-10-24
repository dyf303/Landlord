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
	typedef std::unordered_map<uint32, Player*> PlayerMapType;	
	typedef std::vector<uint32> PlayerVec;
	typedef std::pair<Player*, Player*> twoPlayer;
	typedef std::vector<twoPlayer> vectwoPlayer;

	vectwoPlayer  _vectwoPlayer;
	PlayerMapType _playerMap;
	uint32 _id;
	uint32 _basic_score;
};

#endif