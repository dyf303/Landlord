#include "Player.h"

#include "Log.h"
#include "WorldSession.h"
#include "World.h"

#define PLAYER(left,right) (left !=nullptr ? left:right)

Player::Player(WorldSession* session) :_roomid(0), _left(nullptr), _right(nullptr), _queueFlags(QUEUE_FLAGS_NULL)
, _playerType(PLAYER_TYPE_USER), _start(false), _defaultLandlordUserId(0)
{
	_session = session;

	for (int i = 0; i < CARD_NUMBER; ++i)
		_cards[i] = CARD_TERMINATE;
	for (int i = 0; i < BASIC_CARD; ++i)
		_baseCards[i] = CARD_TERMINATE;

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
}

Player::~Player()
{

}

void Player::Update(uint32 diff)
{
	_expiration -= diff;
	checkOutPlayer();
	checkQueueStatus();
	checkStart();
	checkDealCards();
}

void Player::checkOutPlayer()
{
	if (_left != nullptr && _left->LogOut())
		logOutPlayer(_left->getid());
	if (_right != nullptr && _right->LogOut())
		logOutPlayer(_right->getid());
}

void Player::logOutPlayer(uint32 id)
{

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
	if (_defaultLandlordUserId == 0)
	{
		uint8 iLandlordUserIdx = (rand() % 3);
		switch (iLandlordUserIdx)
		{
		case 0:_defaultLandlordUserId = this->getid(); break;
		case 1:_defaultLandlordUserId = _left->getid(); break;
		case 2:_defaultLandlordUserId = _right->getid(); break;
		}
		_left->setDefaultLandlordUserId(_defaultLandlordUserId);
		_right->setDefaultLandlordUserId(_defaultLandlordUserId);
	}

	return _defaultLandlordUserId;
}

void Player::checkDealCards()
{
	if (_gameStatus == GAME_STATUS_WAIT_DEAL_CARD)
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
	}
}

void Player::sendTwoDesk()
{
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
    
	if (_left == nullptr)
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
	memcpy(_baseCards, baseCards, sizeof(_baseCards));

	_gameStatus = GAME_STATUS_WAIT_DEAL_CARD;
}