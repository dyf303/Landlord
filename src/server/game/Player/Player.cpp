#include "Player.h"

#include "Log.h"
#include "WorldSession.h"

Player::Player(WorldSession* session) :_id(0), _icon_id(0), _sex(0), _gold(0), _level(0), _score(0), _all_Chess(0), _win_chess(0),
_win_Rate(0), _offline_count(0), _start(0), _type(0), _desk_id(0)
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
}

Player::~Player()
{

}