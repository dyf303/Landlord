#include "Room.h"

#include "AiPlayerPool.h"
#include "Player.h"
#include <utility>

#define  RELEASE(player)     if(player->getPlayerType() == PLAYER_TYPE_AI)\
	                          sAiPlayerPool->releasePlayer(player);\
	                           else \
                             delete player;
#define OUT_TWO(player)      if(player->getPlayerType() == PLAYER_TYPE_AI)\
	                           sAiPlayerPool->releasePlayer(player); \
							   else\
							   _OnePlayerList.push_back(player);

#define LOG_OUT_AI(player)   if(player->getPlayerType() & PLAYER_TYPE_AI)\
	                            player->setGameStatus(GAME_STATUS_LOG_OUTED);

Room::Room(uint32 id, uint32 basic_score) :_id(id), _basic_score(basic_score)
{

}

Room::~Room()
{
	_playerMap.clear();
	_OnePlayerList.clear();
	_twoPlayerList.clear();
	_threePlayerList.clear();
}

void Room::Update(const uint32 diff)
{
	for (PlayerMapType::iterator itr = _playerMap.begin(),next; itr != _playerMap.end(); itr = next)
	{
		next = itr;
		next++;
		Player* player = itr->second;

		player->Update(diff);

		if (player->LogOut())
		{
			if (!player->inTheGame())
			{
				_playerMap.erase(itr);
				if (player->idle())/// if not idle, delete player in updateOne Two Three
					RELEASE(player);
			}	
			continue;
		}
		if (player->getGameStatus() == GAME_STATUS_STARTED && player->getQueueFlags() == QUEUE_FLAGS_NULL)
		{
		   player->setQueueFlags(QUEUE_FLAGS_ONE);
		   _OnePlayerList.push_back(player);
		}		
	}
	UpdateOne(diff);
	UpdateTwo(diff);
	UpdateThree(diff);
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

			p0->addPlayer(p1); 
			p1->addPlayer(p2); 
			p2->addPlayer(p0); 

			_threePlayerList.push_back(std::make_pair(std::make_pair(p0, p1),p2));
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

			if (p0 == nullptr)
				break;
			/// add a ai player
			if (p0->expiration())
			{
				Player * p1 = sAiPlayerPool->getAiPlayer(p0->getRoomId());

				p0->addPlayer(p1);
				p1->addPlayer(p0);
				_twoPlayerList.push_back(std::make_pair(p0, p1));
				AddPlayer(p1->getid(), p1, false);

			}
			else
			{
				_OnePlayerList.push_back(p0);
				break;
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
	for (twoPlayerList::iterator itr = _twoPlayerList.begin(),next; itr != _twoPlayerList.end(); itr = next)
	{
		next = itr;
		++next;

		if (LogoutTwo(*itr))
		{
			_twoPlayerList.erase(itr);
			continue;
		}
		Player * p2 = getPlayerFromOne();

		if (itr->first->expiration() || itr->second->expiration() || p2 != nullptr)
		{
			/// add ai player
			Player * p0 = itr->first;
			Player * p1 = itr->second;

			if (p2 == nullptr)
				p2 = sAiPlayerPool->getAiPlayer(itr->first->getRoomId());

			p0->addPlayer(p2); 
			p1->addPlayer(p2);

			_threePlayerList.push_back(std::make_pair(*itr, p2));
			_twoPlayerList.erase(itr);
			AddPlayer(p2->getid(), p2, false);
		}
	}
}

bool Room::LogoutTwo(twoPlayer &twoP)
{
	Player *p0 = twoP.first;
	Player *p1 = twoP.second;
	bool logout = true;
	bool  logout0 = p0->LogOut();
	bool  logout1 = p1->LogOut();

	uint8 logoutStatus = (logout0 ? 1 : 0) | (logout1 ? 1 << 1 : 0);
	switch (logoutStatus)
	{
	case 0:logout = false; break;
	case 1:RELEASE(p0); OUT_TWO(p1); break;
	case 2:OUT_TWO(p0); RELEASE(p1); break;
	case 3:RELEASE(p0); RELEASE(p1) break;
	}
	return logout;
}

void Room::UpdateThree(uint32 diff)
{
	threePlayerList::iterator itr = _threePlayerList.begin();
	for (threePlayerList::iterator itr = _threePlayerList.begin(),next; itr != _threePlayerList.end(); itr = next)
	{
		next = itr;
		next++;
		if (LogoutThree(*itr))
		{
			_threePlayerList.erase(itr);
			continue;
		}
		if (allStart(*itr) && allAtThree(*itr) && allWaitDealCards(*itr))
		{
			dealCards(*itr);
		}
		if (roundOver(*itr))
		{
			_threePlayerList.erase(itr);
		}
	}
}

bool Room::LogoutThree(threePlayer &threeP)
{
	bool logout = true;
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

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

	case 7:RELEASE(p0); RELEASE(p1); RELEASE(p2); break;

	}
	return logout;
}

bool Room::allStart(threePlayer &threeP)
{
	bool allstart = true;
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

	return p0->started() && p1->started() && p2->started();
}

///synchronous player.sendThreeDesk
bool Room::allAtThree(threePlayer &threeP)
{
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

	return p0->getQueueFlags() == QUEUE_FLAGS_THREE 
		&& p1->getQueueFlags() == QUEUE_FLAGS_THREE 
		&& p2->getQueueFlags() == QUEUE_FLAGS_THREE;
}

///To prevent repeat deal cards
bool Room::allWaitDealCards(threePlayer &threeP)
{
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

	return p0->getGameStatus() < GAME_STATUS_DEALING_CARD
		&& p1->getGameStatus() < GAME_STATUS_DEALING_CARD
		&& p2->getGameStatus() < GAME_STATUS_DEALING_CARD;
}

void Room::dealCards(threePlayer &threeP)
{
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

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
		*Cards++ = color << 4 | card;
	}
	*Cards++ = 61;
	*Cards++ = 62;

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

bool Room::roundOver(threePlayer &threeP)
{
	Player *p0 = threeP.first.first;
	Player *p1 = threeP.first.second;
	Player *p2 = threeP.second;

	if (p0->roundOver() && p1->roundOver() && p2->roundOver())
	{
		releaseAiPlayer(threeP);
		return true;
	}
	return false;
}

void Room::releaseAiPlayer(threePlayer &threeP)
{
	LOG_OUT_AI(threeP.first.first);
	LOG_OUT_AI(threeP.first.second);
	LOG_OUT_AI(threeP.second);
}

void Room::AddPlayer(uint32 id, Player *player, bool inOne)
{
	_playerMap[id] = player;
	if (inOne)
	{
	  player->setQueueFlags(QUEUE_FLAGS_ONE);
	  _OnePlayerList.push_back(player);
	}		 
}
