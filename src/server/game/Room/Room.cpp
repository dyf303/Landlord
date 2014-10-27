#include "Room.h"

#include "Player.h"

#include <utility>

typedef std::pair<Player*, Player*> twoPlayer;

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
		if (player->LogOut() && player->idle())
		{
			delete player;
			continue;
		}
		player->Update(diff);
	}
	UpdateOne(diff);
	UpdateTwo(diff);
}

void Room::UpdateOne(uint32 diff)
{
	while (!_OnePlayerList.empty())
	{
		uint32 number = _OnePlayerList.size();
		if (number > 2)
		{
			Player * p0 = getPlayerFromOne();
			Player * p1 = getPlayerFromOne();
			Player * p2 = getPlayerFromOne();
			if (p0 == nullptr || p1 == nullptr || p2 == nullptr)
			{
				continue;
			}

			p0->addPlayer(p1); p0->addPlayer(p2);
			p1->addPlayer(p0); p1->addPlayer(p2);
			p2->addPlayer(p0); p2->addPlayer(p1);

			_threePlayerList.push_back(std::make_pair(p0, std::make_pair(p1, p2)));
		}
		else if (number == 2)
		{
			Player * p0 = getPlayerFromOne();
			Player * p1 = getPlayerFromOne();

			if (p0 == nullptr || p1 == nullptr)
			{
				continue;
			}

			p0->addPlayer(p1);
			p1->addPlayer(p0);
			_twoPlayerList.push_back(std::make_pair(p0, p1));
		}
		else if (number == 1)
		{
			Player * p0 = getPlayerFromOne();

			/// add a ai player
			if (p0 != nullptr && p0->expiration())
			{

			}

		}
	}
}

Player * Room::getPlayerFromOne()
{
	if (_OnePlayerList.empty())
		return nullptr;

	Player * player = _OnePlayerList.front();
	_OnePlayerList.pop_front();

	if (player->LogOut())
	{
		delete player;
		return nullptr;
	}
	return player;
}

void Room::UpdateTwo(uint32 diff)
{
	twoPlayerList::iterator itr = _twoPlayerList.begin();
	for (; itr != _twoPlayerList.end(); ++itr)
	{
		if (LogoutTwo(*itr))
		{
			_twoPlayerList.erase(itr);
			continue;
		}
		if (itr->first->expiration() || itr->second->expiration())
		{
			/// add ai player
		}
	}
}

bool Room::LogoutTwo(twoPlayer &twoP)
{
	Player *p0 = twoP.first;
	Player *p1 = twoP.second;
	bool logout = false;
	if (p0->LogOut() )
	{
		if (!p1->LogOut())
		{
			_OnePlayerList.push_back(p1);
		}
		delete p0;
		logout = true;
	}

	if (p1->LogOut())
	{
		if (!p0->LogOut())
		{
			_OnePlayerList.push_back(p0);
		}
		delete p1;
		logout = true;
	}
	return logout;
}

void Room::UpdateThree(uint32 diff)
{
	threePlayerList::iterator itr = _threePlayerList.begin();
	for (; itr != _threePlayerList.end(); ++itr)
	{
		if (!LogoutThree(*itr) && allStart(*itr))
		{
			dealCards(*itr);
		}
		if (endGame(*itr))
		{
			_threePlayerList.erase(itr);
		}
	}
}

bool Room::LogoutThree(threePlayer &threeP)
{
	bool logout = true;
	Player *p0 = threeP.first;
	Player *p1 = threeP.second.first;
	Player *p2 = threeP.second.second;
	bool  logout0 = p0->LogOut();
	bool  logout1 = p1->LogOut();
	bool  logout2 = p2->LogOut();

	uint8 logoutStatus = (logout0 ? 1 : 0) | (logout1 ? 1 << 1 : 0) | (logout2 ? 1 << 2 : 0);

	switch (logoutStatus)
	{
	case 0: logout = false; break;

	case 1:_twoPlayerList.push_back(std::make_pair(p1, p2)); delete p0; break;

	case 2:	_twoPlayerList.push_back(std::make_pair(p0, p2)); delete p1; break;

	case 3: _OnePlayerList.push_back(p2); delete p0; delete p1; break;

	case 4: _twoPlayerList.push_back(std::make_pair(p0, p1)); delete p2; break;

	case 5:_OnePlayerList.push_back(p1); delete p0; delete p2; break;

	case 6:_OnePlayerList.push_back(p0); delete p1; delete p2; break;

	case 7:delete p0; delete p1; delete p2; break;

	}
	return logout;
}

bool Room::allStart(threePlayer &threeP)
{
	bool allstart = true;
	Player *p0 = threeP.first;
	Player *p1 = threeP.second.first;
	Player *p2 = threeP.second.second;

	return p0->started() && p1->started() && p2->started();
}

void Room::dealCards(threePlayer &threeP)
{
	Player *p0 = threeP.first;
	Player *p1 = threeP.second.first;
	Player *p2 = threeP.second.second;

	uint8 cards[54];
	shuffleCard(cards);
  
	p0->dealCards(cards, cards + 51);
	p1->dealCards(cards + 17, cards + 51);
	p2->dealCards(cards + 34, cards + 51);
}

void Room::shuffleCard(uint8* Cards)
{
	for (uint32 color = 0; color < 4; color++)
	for (uint32 card = 0; card < 13; card++)
	{
		*++Cards = color << 4 | card;
	}
	*++Cards = 61;
	*++Cards = 62;

	uint8 iSwapTmp;
	Cards -= 54;
	for (uint32 iIdx = 0; iIdx < 54; iIdx++)
	{
		uint32 iRandNum = rand() % 54;
		if (iIdx != iRandNum)
		{
			iSwapTmp = Cards[iIdx];
			Cards[iIdx] = Cards[iRandNum];
			Cards[iRandNum] = iSwapTmp;
		}
	}
}

bool Room::endGame(threePlayer &threeP)
{
	Player *p0 = threeP.first;
	Player *p1 = threeP.second.first;
	Player *p2 = threeP.second.second;

	if (p0->endGame() && p1->endGame() && p2->endGame())
	{
		return true;
	}
	return false;
}

void Room::AddPlayer(uint32 id,Player *player)
{
	_playerMap[id] = player;
	_OnePlayerList.push_back(player);
}
