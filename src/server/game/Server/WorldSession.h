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

/// \addtogroup u2w
/// @{
/// \file

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "Common.h"
#include "World.h"
#include "Opcodes.h"
#include "WorldPacket.h"

#include <unordered_set>

class Player;
class Unit;
class WorldPacket;
class WorldSocket;


/// Player session in the World
class WorldSession
{
    public:
        WorldSession(uint32 id, std::shared_ptr<WorldSocket> sock);
        ~WorldSession();

		uint32 GetAccountId() const { return _accountId; }

		void QueuePacket(WorldPacket* new_packet);
        void SendPacket(WorldPacket* packet);
		void KickPlayer();

		bool Update(uint32 diffr);

		std::atomic<int32> m_timeOutTime;

		void UpdateTimeOutTime(uint32 diff)
		{
			m_timeOutTime -= int32(diff);
		}
		void ResetTimeOutTime()
		{
			m_timeOutTime = int32(sWorld->getIntConfig(CONFIG_SOCKET_TIMEOUTTIME));
		}

		bool IsTimeOutTime() const
		{
			return m_timeOutTime <= 0;
		}
		void SendLoginError(uint8 code);
    public:                                                 // opcodes handlers
		void Handle_NULL(WorldPacket& recvPacket);          // not used
		void HandlePlayerLogin(WorldPacket& recvPacket);

    friend class World;
 

    private:

        std::shared_ptr<WorldSocket> _Socket;
        std::string _Address;                // Current Remote Address
		uint32 _accountId;
		Player* _player;

        LockedQueue<WorldPacket*> _recvQueue;

        WorldSession(WorldSession const& right) = delete;
        WorldSession& operator=(WorldSession const& right) = delete;

		uint32 _expireTime;
		bool _forceExit;
};
#endif
/// @}
