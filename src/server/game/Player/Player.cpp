#include "Player.h"

#include "Log.h"
#include "OutCardAI.h"
#include "WorldSession.h"
#include "World.h"

#define PLAYER(left,right) (left !=nullptr ? left:right)

Player::Player(WorldSession* session) 
{
	_session = session;
	initPlayer();
}

void Player::initPlayer()
{
	_roomid = 0; 
	_left = nullptr; 
	_right = nullptr;
	_curOutCardsPlayer = nullptr;
	_queueFlags = QUEUE_FLAGS_NULL;
	_playerType = PLAYER_TYPE_USER;
	_cardType = CARD_TYPE_PASS;
	_curOutCardType = CARD_TYPE_PASS;
	_start= false;
	_defaultGrabLandlordPlayerId= 0;
	_grabLandlordScore= -1;
	_landlordPlayerId =-1;
	_gameStatus = GAME_STATUS_WAIT_START;
	_winGold = 0;
	_outCardsCount = 0;
	_bombCount = 0;
	_aiGameStatus = AI_GAME_STATUS_NULL;

	for (int i = 0; i < 7; ++i)
		_baseCards[i] = CARD_TERMINATE;
	for (int i = 0; i < 24; ++i)
	{
		_outCards[i] = CARD_TERMINATE;
		_curOutCards[i] = CARD_TERMINATE;
		_cards[i] = CARD_TERMINATE;
	}

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
	_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
}

Player::~Player()
{

}

PlayerGameType Player::getPlayerGameType()
{
	if (getLandlordId() == getid())
		return PLAYER_GAME_TYPE_LANDLORD;
	else
		return PLAYER_GAME_TYPE_FARMER;
}

void Player::Update(uint32 diff)
{
	UpdateExpiration(diff);
	UpdateQueueStatus();

	if (_playerType & PLAYER_TYPE_AI)
		UpdateAiDelay(diff);

	UpdateGameStatus();
}

void Player::UpdateExpiration(const uint32 diff)
{
	if (_queueFlags == QUEUE_FLAGS_ONE || _queueFlags == QUEUE_FLAGS_TWO)
	{
		_expiration -= diff;
	}
}

void Player::UpdateQueueStatus()
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

void Player::UpdateGameStatus()
{
	switch (_gameStatus)
	{
	case GAME_STATUS_STARTING:handleWaitStart(); break;

	case GAME_STATUS_DEALING_CARD: handleDealCard(); break;

	case GAME_STATUS_GRABING_LANDLORD:handleGrabLandlord(); break;

	case GAME_STATUS_OUT_CARDING:handleOutCard();break;

	case GAME_STATUS_GETING_LEFTCARD:handleGetLeftPlayerCards(); break;

	case GAME_STATUS_ROUNDOVERING:handleRoundOver();break;

	default: if (_gameStatus & 0x10)handLogOut(); break;
	}
}

void Player::handleWaitStart()
{
	WorldPacket data(CMSG_WAIT_START, 12);
	data.resize(8);
	data << uint32(this->getid());

	senToAll(&data);
	_gameStatus = GAME_STATUS_STARTED;
}

void Player::handleDealCard()
{
	WorldPacket data(SMSG_CARD_DEAL, 24);
	data << getDefaultLandlordUserId();
	data.append((uint8 *)_cards, CARD_NUMBER);
	data.append((uint8 *)_baseCards, BASIC_CARD);

	sendPacket(&data);

	_gameStatus = GAME_STATUS_DEALED_CARD;
}

void Player::handleGrabLandlord()
{
	WorldPacket data(CMSG_GRAD_LANDLORD, 20);

	data << uint32(this->getid());
	data << _grabLandlordScore;
	data << getLandlordId();

	senToAll(&data,true);
	_gameStatus = GAME_STATUS_GRABED_LAND_LORD;
	if (getLandlordId() == getid())
	{
		arraggeCard();
		_gameStatus = GAME_STATUS_START_OUT_CARD;
	}
}

void Player::handleOutCard()
{
	++_outCardsCount;
	if (_cardType == CARD_TYPE_BOMB || _cardType == CARD_TYPE_ROCKET)
		++_bombCount;

	WorldPacket data(CMSG_CARD_OUT, 40);
	data << getid();
	data << uint32(_cardType);
	data.append(_outCards, 24);

	sOutCardAi->updateCardsFace(_cards, _outCards);
	UpdateCurOutCardsInfo(_cardType, _outCards, this, true);
	_gameStatus = GAME_STATUS_OUT_CARDED;
	_right->setGameStatus(GAME_STATUS_START_OUT_CARD);
	senToAll(&data, true);

	checkRoundOver();
}

void Player::handleGetLeftPlayerCards()
{
	WorldPacket data(CMSG_REQUEST_CARDS_LEFT, 56);
	data.append(_left->_cards, 24);
	data.append(_right->_cards, 24);

	sendPacket(&data);
	_gameStatus = GAME_STATUS_GETED_LEFTCARD;
}

void Player::handleRoundOver()
{
	UpdatePlayerData();

	WorldPacket data(CMSG_ROUND_OVER, 64);

	//data.append((uint8 *)&_playerInfo, 152);
	data << _left->_winGold;
	data << _right->_winGold;
	data << _winGold;
	data << (uint32)30;

	data.append(_left->_cards, 24);
	data.append(_right->_cards, 24);

	sendPacket(&data);

	_gameStatus = GAME_STATUS_ROUNDOVERED;

	if (_left->getGameStatus() == GAME_STATUS_ROUNDOVERED &&
		_right->getGameStatus() == GAME_STATUS_ROUNDOVERED)
	{
		_left->resetGame();
		_right->resetGame();
		resetGame();
	}	  
}

void Player::handLogOut()
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

	senToAll(&data,true);

	if (logoutStatus == 4)
	{
		if (_left->getPlayerType() == PLAYER_TYPE_USER || _right->getPlayerType() == PLAYER_TYPE_USER)
		{
			_gameStatus = GameStatus(0x0f & _gameStatus);
			_playerType = PLAYER_TYPE_REPLACE_AI;
			setSession(nullptr);
			aiHandGame();
			return;
		}
	}

	notifyOther();
	if (_playerType == PLAYER_TYPE_USER)
		GetSession()->setPlayer(nullptr);
	_gameStatus = GAME_STATUS_LOG_OUTED;
}

void Player::checkRoundOver()
{
	if (0 == sOutCardAi->getCardsNumber(_cards))
	{
		int32 multiple = calcMultiple();

		setGameStatus(GAME_STATUS_ROUNDOVERING);
		calcWinGold(this, multiple);

		_left->setGameStatus(GAME_STATUS_ROUNDOVERING);
		_left->calcWinGold(this, multiple);

		_right->setGameStatus(GAME_STATUS_ROUNDOVERING);
		_right->calcWinGold(this, multiple);
	}
}

void Player::UpdateCurOutCardsInfo(CardType cardType, uint8 * outCards, Player *outCardsPlayer, bool updateOther/* = false*/)
{
	if (cardType != CARD_TYPE_PASS)
	{
		memcpy(_curOutCards, outCards, sizeof(_curOutCards));
		_curOutCardType = cardType;
		_curOutCardsPlayer = outCardsPlayer;
		if (updateOther)
		{
			_left->UpdateCurOutCardsInfo(cardType, outCards, outCardsPlayer);
			_right->UpdateCurOutCardsInfo(cardType, outCards, outCardsPlayer);
		}
	}
}

void Player::notifyOther()
{
	if (_left != nullptr)
		_left->_right = nullptr;
	if (_right != nullptr)
		_right->_left = nullptr;
}

void Player::arraggeCard()
{
	memcpy(_cards + CARD_NUMBER, _baseCards, BASIC_CARD);
	sOutCardAi->arraggeCard(_cards, CARD_NUMBER + BASIC_CARD);
	sOutCardAi->arraggeCard(_left->_cards, CARD_NUMBER);
	sOutCardAi->arraggeCard(_right->_cards, CARD_NUMBER);
	setCurOutCardPlayer(this);
	_left->setCurOutCardPlayer(this);
	_right->setCurOutCardPlayer(this);
}

void Player::senToAll(WorldPacket* packet,bool bSelf/* = false*/)
{
	if (bSelf)
	   sendPacket(packet);

	if (_left != nullptr)
		_left->sendPacket(packet);

	if (_right != nullptr)
		_right->sendPacket(packet);
}

void Player::sendPacket(WorldPacket* packet)
{
	if (_playerType == PLAYER_TYPE_USER)
		GetSession()->SendPacket(packet);
	else
		aiRecvPacket(packet);
}

void Player::logOutPlayer()
{
	_gameStatus = GameStatus(_gameStatus | GAME_STATUS_LOG_OUTING);
	handLogOut();
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

void Player::resetGame()
{
	GameStatus curGameStatus = getGameStatus();
	PlayerType curPlayerType = getPlayerType();
	initPlayer();
	setGameStatus(curGameStatus);
	setPlayerType(curPlayerType);
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

	WorldPacket data(SMSG_DESK_TWO, 1 + 152);

	if (_left != nullptr)
	{
		data << uint8(LEFT);
	}
	else
	{
		data << uint8(RITHT);
	}

	Player *player = _right != nullptr ? _right : _left;

	data.append((uint8 *)player->getPlayerInfo(), sizeof(PlayerInfo));

	GetSession()->SendPacket(&data);
}

void Player::sendThreeDesk()
{
	if (getPlayerType() != PLAYER_TYPE_USER)
		return;

	WorldPacket data(SMSG_DESK_THREE, 152 + 152);

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
	memcpy(_cards, cards, CARD_NUMBER);
	memcpy(_baseCards, baseCards, BASIC_CARD);

	_gameStatus = GAME_STATUS_DEALING_CARD;
}

bool Player::bHaveSpring()
{
	if (getid() == _landlordPlayerId)
	{
		if (_left->_outCardsCount == 0 && _right->_outCardsCount == 0)
			return true;
	}
	else
	{
		Player * landlord = _left->getid() == _landlordPlayerId ? _left : _right;
		if (landlord->_outCardsCount == 1)
			return true;
	}
	return false;
}

int32 Player::calcMultiple()
{
	uint32 Spring = bHaveSpring() ? 2 : 1;
	int32 maxScore = std::max(_grabLandlordScore,std::max(_left->_grabLandlordScore,_right->_grabLandlordScore));

	return maxScore * pow(2, _bombCount) * Spring;
}

void Player::calcWinGold(Player * winPlayer, int32 Multiple)
{
	int32 winGold = Multiple * 100;

	Player * landlord = _left->getid() == _landlordPlayerId ? _left : _right;

	bool bWin = winPlayer->getPlayerGameType() == getPlayerGameType();

	if (landlord == this)//landlord
	{
		if (bWin)
		{
			_winGold = std::min((int32)_left->getPlayerInfo()->gold, winGold) + std::min((int32)_right->getPlayerInfo()->gold, winGold);
		}
		else
		{
			_winGold = -std::min((int32)_playerInfo.gold, 2 * winGold);
		}
	}
	else //farmer
	{
		if (bWin)
		{
			_winGold = std::min((int32)landlord->getPlayerInfo()->gold / 2, winGold);
		}
		else
		{
			_winGold = -std::min((int32)_playerInfo.gold, winGold);
		}
	}
}