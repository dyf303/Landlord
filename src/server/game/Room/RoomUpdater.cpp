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

#include <mutex>
#include <condition_variable>

#include "RoomUpdater.h"
#include "Room.h"


class RoomUpdateRequest
{
    private:

        Room& _room;
        RoomUpdater& _updater;
        uint32 _diff;

    public:

		RoomUpdateRequest(Room& m, RoomUpdater& u, uint32 d)
			: _room(m), _updater(u), _diff(d)
        {
        }

        void call()
        {
			_room.Update(_diff);
			_updater.update_finished();
        }
};

void RoomUpdater::activate(size_t num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        _workerThreads.push_back(std::thread(&RoomUpdater::WorkerThread, this));
    }
}

void RoomUpdater::deactivate()
{
    _cancelationToken = true;

    wait();

    _queue.Cancel();

    for (auto& thread : _workerThreads)
    {
        thread.join();
    }
}

void RoomUpdater::wait()
{
    std::unique_lock<std::mutex> lock(_lock);

    while (pending_requests > 0)
        _condition.wait(lock);

    lock.unlock();
}

void RoomUpdater::schedule_update(Room& room, uint32 diff)
{
    std::lock_guard<std::mutex> lock(_lock);

    ++pending_requests;

	_queue.Push(new RoomUpdateRequest(room, *this, diff));
}

bool RoomUpdater::activated()
{
    return _workerThreads.size() > 0;
}

void RoomUpdater::update_finished()
{
    std::lock_guard<std::mutex> lock(_lock);

    --pending_requests;

    _condition.notify_all();
}

void RoomUpdater::WorkerThread()
{
    while (1)
    {
        RoomUpdateRequest* request = nullptr;

        _queue.WaitAndPop(request);

        if (_cancelationToken)
            return;

        request->call();

        delete request;
    }
}
