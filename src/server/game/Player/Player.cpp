#include "Player.h"

#include "Log.h"
#include "WorldSession.h"
#include "World.h"

Player::Player(WorldSession* session) :_id(0), _icon_id(0), _sex(0), _gold(0), _level(0), _score(0), _all_Chess(0), _win_chess(0),
_win_Rate(0), _offline_count(0), _start(0), _type(0), _desk_id(0), left(nullptr), right(nullptr)
{
	for (uint8 i = 0; i < PROPS_COUNT; ++i)
	{
		_props_count[i] = 0;
	}
	for (uint8 i = 0; i < NAME_LENGTH; ++i)
	{
		_account[i] = 0;
		_name[i] = 0;
		_nick_name[i] = 0;
	}
	_session = session;
	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
}

Player::~Player()
{

}

void Player::Update(uint32 diff)
{
	_expiration -= diff;
}

void Player::loadData(PlayerInfo &pInfo)
{
	_id            = pInfo.id;
	_icon_id       = pInfo.icon_id;
	_sex           = pInfo.sex;
	_gold          = pInfo.gold;
	_level         = pInfo.level;
	_score         = pInfo.score;
	_all_Chess     = pInfo.all_Chess;
	_win_chess     = pInfo.win_chess;
	_win_Rate      = pInfo.win_Rate;
	_offline_count = pInfo.offline_count;
	_start         = pInfo.start;
	_type          = pInfo.type;
	_desk_id       = pInfo.desk_id;
	memcpy(_props_count, pInfo.props_count, sizeof(_props_count));
	memcpy(_account, pInfo.account, sizeof(_account));
	memcpy(_name, pInfo.name, sizeof(_name));
	memcpy(_nick_name, pInfo.nick_name, sizeof(_nick_name));

}

void Player::addPlayer(Player *player)
{
	ASSERT(player != nullptr);
	if (left == nullptr)
		left = player;
	else
		right = player;

	_expiration = sWorld->getIntConfig(CONFIG_WAIT_TIME);
}

void Player::dealCards(uint8 * cards, uint8 * baseCards)
{
	memcpy(_cards, cards, sizeof(_cards));
	memcpy(_baseCards, baseCards, sizeof(_baseCards));
}