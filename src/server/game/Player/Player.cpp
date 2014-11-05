#include "Player.h"

#include "Log.h"
#include "OutCardAI.h"
#include "WorldSession.h"
#include "World.h"

#define PLAYER(left,right) (left !=nullptr ? left:right)

Player::Player(WorldSession* session) :_roomid(0), _left(nullptr), _right(nullptr), _queueFlags(QUEUE_FLAGS_NULL)
, _playerType(PLAYER_TYPE_USER), _start(false), _defaultGrabLandlordPlayerId(0), _grabLandlordScore(-1), _landlordPlayerId(-1)
, _gameStatus(GAME_STATUS_WAIT_START), _winGold(0)
{
	_session = session;

	for (int i = 0; i < CARD_NUMBER; ++i)
		_cards[i] = CARD_TERMINATE;
	for (int i = 0; i < BASIC_CARD; ++i)
		_baseCards[i] = CARD_TERMINATE;

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
	_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
}

Player::~Player()
{

}

void Player::Update(uint32 diff)
{
	if (_queueFlags == QUEUE_FLAGS_ONE || _queueFlags == QUEUE_FLAGS_TWO)
	  _expiration -= diff;

	UpdateAiDelay(diff);

	checkOutPlayer();
	checkQueueStatus();
	checkStart();
	checkDealCards();
	checkGrabLandlord();
	checkOutCard();
	checkRoundOver();
}

void Player::UpdateAiDelay(const uint32 diff)
{
	if (!(getPlayerType() & PLAYER_TYPE_AI))
		return;

	if (_gameStatus == GAME_STATUS_GRABING_LANDLORD || _gameStatus == GAME_STATUS_OUT_CARDING)
	{
		_aiDelay -= diff;
	}
}

void Player::checkOutPlayer()
{
	if ((_gameStatus & 0xf0) == GAME_STATUS_LOG_OUTING)
	{
		uint32 logoutStatus;
		GameStatus preGameStatus = GameStatus(_gameStatus & 0x0f);
		if (preGameStatus > GAME_STATUS_DEALING_CARD && preGameStatus < GAME_STATUS_ROUNDOVERING)
		{
			logoutStatus = 4;
		}
		else
		{
			logoutStatus = 2;
		}

		WorldPacket data(CMSG_LOG_OUT, 16);
		
		data.resize(8);
		data << getid();
		data << logoutStatus;

		if (getPlayerType() == PLAYER_TYPE_USER)
			GetSession()->SendPacket(&data);

		if (_left != nullptr && _left->getPlayerType() == PLAYER_TYPE_USER )
			_left->GetSession()->SendPacket(&data);

		if (_right != nullptr && _right->getPlayerType() == PLAYER_TYPE_USER )
			_right->GetSession()->SendPacket(&data);

		if (logoutStatus == 4)
		{
			if (_left->getPlayerType() == PLAYER_TYPE_USER || _right->getPlayerType() == PLAYER_TYPE_USER)
			{
				_gameStatus = GameStatus(0x0f & _gameStatus);
			}
			else
			{
				_left->setGameStatus(GAME_STATUS_LOG_OUTING);
				_right->setGameStatus(GAME_STATUS_LOG_OUTING);

				_gameStatus = GAME_STATUS_LOG_OUTED;
			}
			_playerType = PLAYER_TYPE_REPLACE_AI;
		}			
		else
		{
			if (_left != nullptr)
			{
				_left->_right = nullptr;
				if (_left->getPlayerType() == PLAYER_TYPE_AI)
				{
					_left->setGameStatus(GAME_STATUS_LOG_OUTING);
					_left->checkOutPlayer();
				}
				
			}
			if (_right != nullptr)
			{
				_right->_left = nullptr;
				if (_right->getPlayerType() == PLAYER_TYPE_AI)
				{
					_right->setGameStatus(GAME_STATUS_LOG_OUTING);
					_right->checkOutPlayer();
				}									
			}				
			_gameStatus = GAME_STATUS_LOG_OUTED;
		}
		
		if (_session != nullptr)
		  GetSession()->setPlayer(nullptr);
		_session = nullptr;
	}
}

void Player::logOutPlayer()
{
	_gameStatus = GameStatus(_gameStatus | GAME_STATUS_LOG_OUTING);
	checkOutPlayer();
}

void Player::checkQueueStatus()
{
	if (_left != nullptr && _right != nullptr)
	{
		if (_queueFlags != QUEUE_FLAGS_THREE)
		{
			_queueFlags = QUEUE_FLAGS_THREE;
			sendThreeDesk();
		}
	}
	else if (_left != nullptr || _right != nullptr)
	{
		if (_queueFlags != QUEUE_FLAGS_TWO)
		{
			_queueFlags = QUEUE_FLAGS_TWO;
			sendTwoDesk();
		}
	}

}

void Player::checkStart()
{
	if (_gameStatus == GAME_STATUS_STARTING)
	{
		if (_left != nullptr && _left->getPlayerType() == PLAYER_TYPE_USER)
		{
			WorldPacket data(CMSG_WAIT_START, 12);
			data.resize(8);
			data << uint32(this->getid());

			_left->GetSession()->SendPacket(&data);
		}

		if (_right != nullptr && _right->getPlayerType() == PLAYER_TYPE_USER)
		{
			WorldPacket data(CMSG_WAIT_START, 12);
			data.resize(8);
			data << uint32(this->getid());

			_right->GetSession()->SendPacket(&data);
		}
		_gameStatus = GAME_STATUS_STARTED;
	}
}

uint32 Player::getDefaultLandlordUserId()
{
	if (_defaultGrabLandlordPlayerId == 0)
	{
		uint8 iLandlordUserIdx = (rand() % 3);
		switch (iLandlordUserIdx)
		{
		case 0:_defaultGrabLandlordPlayerId = this->getid(); break;
		case 1:_defaultGrabLandlordPlayerId = _left->getid(); break;
		case 2:_defaultGrabLandlordPlayerId = _right->getid(); break;
		}
		_left->setDefaultLandlordUserId(_defaultGrabLandlordPlayerId);
		_right->setDefaultLandlordUserId(_defaultGrabLandlordPlayerId);
	}

	return _defaultGrabLandlordPlayerId;
}

void Player::checkDealCards()
{
	if (_gameStatus == GAME_STATUS_DEALING_CARD)
	{
		if (getPlayerType() == PLAYER_TYPE_USER)
		{
			WorldPacket data(SMSG_CARD_DEAL, 36);
			data.resize(8);
			data << getDefaultLandlordUserId();
			data.append((char *)_cards, CARD_NUMBER);
			data.append((char *)_baseCards, BASIC_CARD);

			GetSession()->SendPacket(&data);
		}
		_gameStatus = GAME_STATUS_DEALED_CARD;

		if (getDefaultLandlordUserId() == getid() && (getPlayerType() & PLAYER_TYPE_AI))
			_gameStatus = GAME_STATUS_GRABING_LANDLORD;
	}
}

uint32 Player::aiGrabLandlord()
{
	uint32 leftGrabScore = _left->getGrabLandlordScore();
	uint32 rightGrabScore = _right->getGrabLandlordScore();

	uint32 maxScore = std::max(leftGrabScore, rightGrabScore);

	if (leftGrabScore == -1 && rightGrabScore == -1)
		return (rand() % 4);
	else if (maxScore == 0)
		return 1;
	else 
		return 0;
}

int32 Player::getLandlordId()
{
	if (_landlordPlayerId != -1)
		return _landlordPlayerId;

	int32 leftGrabScore = _left->getGrabLandlordScore();
	int32 rightGrabScore = _right->getGrabLandlordScore();
	int32 maxScore = std::max(std::max(_grabLandlordScore, leftGrabScore), rightGrabScore);

	if ((_grabLandlordScore != -1 && leftGrabScore != -1 && rightGrabScore != -1) || maxScore == 3)
	{	
		if (maxScore == _grabLandlordScore)
			_landlordPlayerId = getid();
		else if (maxScore == leftGrabScore)
			_landlordPlayerId = _left->getid();
		else if (maxScore == rightGrabScore)
			_landlordPlayerId =  _right->getid();
	}
	return _landlordPlayerId;
}

void Player::checkGrabLandlord()
{
	if (_gameStatus != GAME_STATUS_DEALED_CARD && _gameStatus != GAME_STATUS_GRABING_LANDLORD)
		return;

	if (_gameStatus == GAME_STATUS_DEALED_CARD)
	{
		if (getPlayerType() & PLAYER_TYPE_AI && (_defaultGrabLandlordPlayerId == getid()
			|| _left->getGameStatus() == GAME_STATUS_GRABED_LAND_LORD))
		{
			_gameStatus = GAME_STATUS_GRABING_LANDLORD;
		}
	}

	if (_gameStatus == GAME_STATUS_GRABING_LANDLORD)
	{
		do 
		{
			if (getPlayerType() & PLAYER_TYPE_AI)
			{
				if (_aiDelay > 0)
					break;
				else
				{
					_grabLandlordScore = aiGrabLandlord();
					_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
				}
			}
		
		    WorldPacket data(CMSG_GRAD_LANDLORD, 20);

			data.resize(8);
			data << uint32(this->getid());
			data << _grabLandlordScore;
			data << getLandlordId();

			if (getPlayerType() == PLAYER_TYPE_USER)
				GetSession()->SendPacket(&data);

			if (_left->getPlayerType() == PLAYER_TYPE_USER)
				_left->GetSession()->SendPacket(&data);

			if (_right->getPlayerType() == PLAYER_TYPE_USER)
				_right->GetSession()->SendPacket(&data);

		    _gameStatus = GAME_STATUS_GRABED_LAND_LORD;
		} while (0);
	}

	if (_gameStatus == GAME_STATUS_GRABED_LAND_LORD)
	{
		if (getGrabLandlordScore() == 0 && _left->getGrabLandlordScore() == 0 && _right->getGrabLandlordScore() == 0)
		{
			_defaultGrabLandlordPlayerId = 0;
			_grabLandlordScore = -1;
			_gameStatus = GAME_STATUS_STARTED;
		}
		if (getLandlordId() != -1)
		{
			beginOutCard();
		}
	}
}

void Player::beginOutCard()
{
	_left->setLandlordId(_landlordPlayerId);
	_right->setLandlordId(_landlordPlayerId);

	setGameStatus(GAME_STATUS_WAIT_OUT_CARD);
	_left->setGameStatus(GAME_STATUS_WAIT_OUT_CARD);
	_right->setGameStatus(GAME_STATUS_WAIT_OUT_CARD);

	//if (_landlordPlayerId == getid() && getPlayerType() & PLAYER_TYPE_AI)
	//	setGameStatus(GAME_STATUS_OUT_CARDING);
	//else if (_landlordPlayerId == _left->getid() && _left->getPlayerType() & PLAYER_TYPE_AI)
	//	_left->setGameStatus(GAME_STATUS_OUT_CARDING);
	//else if (_landlordPlayerId == _right->getid() && _right->getPlayerType() & PLAYER_TYPE_AI)
	//	_right->setGameStatus(GAME_STATUS_OUT_CARDING);
}

void Player::checkOutCard()
{
	if (_gameStatus < GAME_STATUS_WAIT_OUT_CARD || _gameStatus > GAME_STATUS_OUT_CARDED)
		return;
	if (getPlayerType() & PLAYER_TYPE_AI && (_gameStatus == GAME_STATUS_WAIT_OUT_CARD && getLandlordId() == getid()
		|| _left->getGameStatus() == GAME_STATUS_OUT_CARDED))
	{
		_gameStatus = GAME_STATUS_OUT_CARDING;
	}

	if (_gameStatus == GAME_STATUS_OUT_CARDING)
	{
		do 
		{
			if (getPlayerType() & PLAYER_TYPE_AI)
			{
				if (_aiDelay > 0)
					break;
				else
				{
					/// ai out cards
					sOutCardAi->OutCard(this);
					_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
				}
			}
			WorldPacket data(CMSG_CARD_OUT, 40);
			data.resize(8);
			data << getid();
			data << uint32(_cardType);
			data.append(_outCards, 24);

			if (getPlayerType() == PLAYER_TYPE_USER)
				GetSession()->SendPacket(&data);

			if (_left->getPlayerType() == PLAYER_TYPE_USER)
				_left->GetSession()->SendPacket(&data);

			if (_right->getPlayerType() == PLAYER_TYPE_USER)
				_right->GetSession()->SendPacket(&data);

			_gameStatus = GAME_STATUS_OUT_CARDED;

			sOutCardAi->updateCardsFace(_cards, _outCards);

			if (_right->getPlayerType() & PLAYER_TYPE_AI)
				_right->setGameStatus(GAME_STATUS_OUT_CARDING);
		} while (0);
	}
	if (_gameStatus == GAME_STATUS_OUT_CARDED && _right->getGameStatus() == GAME_STATUS_OUT_CARDED)
		_gameStatus = GAME_STATUS_WAIT_OUT_CARD;
}

void Player::checkRoundOver()
{
	if (_gameStatus == GAME_STATUS_ROUNDOVERING)
	{
		UpdatePlayerData();
		
		WorldPacket data(CMSG_ROUND_OVER, 160);

		data.resize(8);
		data.append((uint8 *)&_playerInfo, 152);

		if (getPlayerType() == PLAYER_TYPE_USER)
			GetSession()->SendPacket(&data);

		_gameStatus = GAME_STATUS_ROUNDOVERED;

		if (_left->getPlayerType() & PLAYER_TYPE_AI && _left->getGameStatus() != GAME_STATUS_ROUNDOVERED)
			_left->setGameStatus(GAME_STATUS_ROUNDOVERING);
		if (_right->getPlayerType() & PLAYER_TYPE_AI && _right->getGameStatus() != GAME_STATUS_ROUNDOVERED)
		{
			_right->setGameStatus(GAME_STATUS_ROUNDOVERING);
			_right->checkRoundOver();
		}
			
	}
	if (_gameStatus == GAME_STATUS_ROUNDOVERED)
	{
		resetGame();
	}
}

void Player::resetGame()
{
	for (int i = 0; i < CARD_NUMBER; ++i)
		_cards[i] = CARD_TERMINATE;
	for (int i = 0; i < BASIC_CARD; ++i)
		_baseCards[i] = CARD_TERMINATE;

	if (getPlayerType() & PLAYER_TYPE_USER )
		_queueFlags = QUEUE_FLAGS_NULL;

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
	_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
	_left = nullptr;
	_right = nullptr;
	_start = false;
	_defaultGrabLandlordPlayerId = 0; 
	_grabLandlordScore = -1;
	_landlordPlayerId = -1;
	_winGold = 0;
}

void Player::UpdatePlayerData()
{
	_playerInfo.gold += _winGold;
	if (_playerInfo.gold < 0)
		_playerInfo.gold = 0;

	_playerInfo.all_Chess++;

	if (_winGold > 0)
		_playerInfo.win_chess++;

	_playerInfo.win_Rate = 100 * _playerInfo.win_chess / (float)_playerInfo.all_Chess;

	uint32 doubleScore = calcDoubleScore();
	_playerInfo.score += _winGold > 0 ? (_roomid + 1) * sWorld->getIntConfig(CONFIG_BASICSCORE) * doubleScore : 0;

	UpdatePlayerLevel();
}

void Player::UpdatePlayerLevel()
{
	static uint32 LevelTable[] = 
	{ 5000, 10000, 50000, 100000, 300000, 800000, 1500000, 
	  2000000, 5000000, 10000000, 15000000, 30000000, 50000000, 100000000 };

	uint32 level = 0;
	uint32 score = _playerInfo.score;

	while (score >= LevelTable[level])
		level++;

	_playerInfo.level = level;
}

uint32 Player::calcDoubleScore()
{
	uint32 doubleScore = 1;
	if (_playerInfo.props_count[0] > 0 || _playerInfo.props_count[1] == -1)//Ë«±¶¾­Ñé¿¨
	{
		doubleScore *= 2;
	}
	if (_playerInfo.props_count[13] > 0 || _playerInfo.props_count[14] > 0 || _playerInfo.props_count[15] == -1)//VIP
	{
		doubleScore *= 2;
	}

	return doubleScore;
}

void Player::sendTwoDesk()
{
	if (getPlayerType() != PLAYER_TYPE_USER)
		return;

	WorldPacket data(SMSG_DESK_TWO, 316);

	data.resize(8);
	data << uint32(1);
	
	Player *player = _right != nullptr ? _right : _left;

	if (_left)
		data.resize(8 + 4 + 152);

	data.append((uint8 *)player->getPlayerInfo(),sizeof(PlayerInfo));

	if (_right)
		data.resize(8 + 4 + 152 + 152);

	GetSession()->SendPacket(&data);
}

void Player::sendThreeDesk()
{
	if (getPlayerType() != PLAYER_TYPE_USER)
		return;

	WorldPacket data(SMSG_DESK_THREE, 316);

	data.resize(8);
	data << uint32(1);
	data.append((uint8 *)_left->getPlayerInfo(), sizeof(PlayerInfo));
	data.append((uint8 *)_right->getPlayerInfo(), sizeof(PlayerInfo));

	GetSession()->SendPacket(&data);
}

void Player::loadData(PlayerInfo &pInfo)
{
	memcpy(&_playerInfo, &pInfo, sizeof(PlayerInfo));
}

void Player::addPlayer(Player *player)
{
	ASSERT(player != nullptr);

	if (player == _left || player == _right)
		return;
    
	if (_left == nullptr )
	{
		_left = player;
		player->setRightPlayer(this);
	}
	else if (_right == nullptr)
	{
		_right = player;
		player->setLeftPlayer(this);
	}

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
}

void Player::dealCards(uint8 * cards, uint8 * baseCards)
{
	memcpy(_cards, cards, sizeof(_cards));
	memcpy(_baseCards, baseCards, 3/*sizeof(_baseCards)*/);

	_gameStatus = GAME_STATUS_DEALING_CARD;
}