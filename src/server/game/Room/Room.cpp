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
		Player* player0 = itr->second;
		player0->Update(diff);

		if (++itr != _playerMap.begin())
		{
			Player* player1 = itr->second;
			player1->Update(diff);
			if (++itr != _playerMap.begin())
			{
				Player* player2 = itr->second;
				player2->Update(diff);
			}
			else
			{

			}
		}
		else
		{
			if (player0->expiration())
			{
				/// add ai
			}
		}
		
	}
}

void Room::AddPlayer(uint32 id,Player *player)
{
	_playerMap[id] = player;
}
