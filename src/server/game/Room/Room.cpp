#include "Room.h"

#include "Player.h"

Room::Room(uint32 id, uint32 basic_score) :_id(id), _basic_score(basic_score)
{

}

Room::~Room()
{

}

void Room::Update(const uint32 diff)
{
	for (PlayerMapType::iterator itr = _playerMap.begin(); itr != _playerMap.end(); ++itr)
	{
		Player* player = itr->second;
		player->Update(diff);
	}
	UpdateOne(diff);
	UpdateTwo(diff);
}

void Room::UpdateOne(uint32 diff)
{
	uint32 number = _OnePlayerList.size();
	if (number > 2)
	{
		Player * p0 = _OnePlayerList.front();
		_OnePlayerList.pop_front();
		Player * p1 = _OnePlayerList.front();
		_OnePlayerList.pop_front();
		Player * p2 = _OnePlayerList.front();
		_OnePlayerList.pop_front();

		p0->addPlayer(p1); p0->addPlayer(p2);
		p1->addPlayer(p0); p1->addPlayer(p2);
		p2->addPlayer(p0); p2->addPlayer(p1);

		UpdateOne(diff);
	}
	else if (number == 2)
	{
		Player * p0 = _OnePlayerList.front();
		_OnePlayerList.pop_front();
		Player * p1 = _OnePlayerList.front();
		_OnePlayerList.pop_front();

		p0->addPlayer(p1); 
		p1->addPlayer(p0); 
	}
	else if (number == 1)
	{
		Player * p0 = _OnePlayerList.front();
		_OnePlayerList.pop_front();
		/// add a ai player
	}
}

void Room::UpdateTwo(uint32 diff)
{

}

void Room::AddPlayer(uint32 id,Player *player)
{
	_playerMap[id] = player;
	_OnePlayerList.push_back(player);
}
