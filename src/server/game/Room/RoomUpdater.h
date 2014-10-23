/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ROOM_UPDATER_H_
#define _ROOM_UPDATER_H_

#include "Define.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include "ProducerConsumerQueue.h"

class RoomUpdateRequest;
class Room;

class RoomUpdater
{
    public:

		RoomUpdater() : _cancelationToken(false), pending_requests(0) {}
		~RoomUpdater() { };

		friend class RoomUpdateRequest;

        void schedule_update(Room& map, uint32 diff);

        void wait();

        void activate(size_t num_threads);

        void deactivate();

        bool activated();

    private:

        ProducerConsumerQueue<RoomUpdateRequest*> _queue;

        std::vector<std::thread> _workerThreads;
        std::atomic<bool> _cancelationToken;

        std::mutex _lock;
        std::condition_variable _condition;
        size_t pending_requests;

        void update_finished();

        void WorkerThread();
};

#endif //_ROOM_UPDATER_H_
