
#ifndef _ROOMMANAGER_H
#define _ROOMMANAGER_H


#include "Room.h"
#include "RoomUpdater.h"

class Player;
class Transport;
struct TransportCreatureProto;

class RoomManager
{
    public:
		static RoomManager* instance()
        {
			static RoomManager instance;
            return &instance;
        }

        void Initialize(void);
		void InitRooms();
        void Update(uint32);

		uint32 GetNumPlayers();
		Player * getPlayer(uint32 id);
		void AddPlayer(uint32 roomid,Player * player);
        void UnloadAll();

    private:
        typedef std::unordered_map<uint32, Room*> RoomMapType;

		RoomManager();
		~RoomManager();

		RoomMapType _roomMap;
		IntervalTimer _i_timer;
        std::mutex _roomsLock;
        RoomUpdater _updater;

		uint32 _num_rooms;
		uint32 _basic_score;
};
#define sRoomMgr RoomManager::instance()
#endif
