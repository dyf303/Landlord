

#include "RoomManager.h"

#include "Log.h"
#include "Config.h"
#include "World.h"
#include "WorldPacket.h"
#include "Player.h"
#include "WorldSession.h"
#include "Opcodes.h"

RoomManager::RoomManager()
{
	_i_timer.SetInterval(sWorld->getIntConfig(CONFIG_INTERVAL_ROOMUPDATE));
}

RoomManager::~RoomManager() { }

void RoomManager::Initialize()
{
	_num_rooms= sWorld->getIntConfig(CONFIG_NUMBERROOMS);
	_basic_score= sWorld->getIntConfig(CONFIG_BASICSCORE);

	InitRooms();

   uint32 num_threads(sWorld->getIntConfig(CONFIG_NUMTHREADS));

    if (num_threads > 0)
        _updater.activate(num_threads);
}

void RoomManager::InitRooms()
{
	for (uint32 id = 0; id < _num_rooms; ++id)
	{
		Room *room = new Room(id, _basic_score * (id + 1));
		_roomMap[id] = room;
	}
}

void RoomManager::Update(uint32 diff)
{
    _i_timer.Update(diff);
    if (!_i_timer.Passed())
        return;

	RoomMapType::iterator iter = _roomMap.begin();
	for (; iter != _roomMap.end(); ++iter)
    {
        if (_updater.activated())
            _updater.schedule_update(*iter->second, uint32(_i_timer.GetCurrent()));
        else
            iter->second->Update(uint32(_i_timer.GetCurrent()));
    }
    if (_updater.activated())
        _updater.wait();

    _i_timer.SetCurrent(0);
}

void RoomManager::UnloadAll()
{
	for (RoomMapType::iterator iter = _roomMap.begin(); iter != _roomMap.end();)
    {
       // iter->second->UnloadAll();
        delete iter->second;
		_roomMap.erase(iter++);
    }

    if (_updater.activated())
        _updater.deactivate();

}

uint32 RoomManager::GetNumPlayers()
{
	std::lock_guard<std::mutex> lock(_roomsLock);

    uint32 ret = 0;
	for (RoomMapType::iterator itr = _roomMap.begin(); itr != _roomMap.end(); ++itr)
    {
        Room* room = itr->second;
     ///  ??????????????????????????????
    }
    return ret;
}

Player* RoomManager::getPlayer(uint32 id)
{
	for (RoomMapType::iterator itr = _roomMap.begin(); itr != _roomMap.end(); ++itr)
    {
        Room* room = itr->second;
		Player * player;

		player = room->findPlayer(id);
		if (player != nullptr)
			return player;
    }
	return nullptr;
}

void RoomManager::AddPlayer(uint32 roomid, Player * player)
{
	RoomMapType::iterator itr = _roomMap.find(roomid);

	if (itr != _roomMap.end())
	  itr->second->AddPlayer(player->getid(),player);
}
