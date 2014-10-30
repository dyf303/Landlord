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

/** \file
    \ingroup u2w
*/

#include "WorldSocket.h"
#include "Config.h"
#include "Common.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "RoomManager.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"

namespace 
{
	char const * DefaultPlayerName = "<none>";
} // namespace

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, std::shared_ptr<WorldSocket> sock):
    _Socket(sock),
    _accountId(id),
	_player(nullptr),
	_forceExit(false)
{
    if (sock)
    {
        _Address = sock->GetRemoteIpAddress().to_string();
		ResetTimeOutTime();
    }
}

/// WorldSession destructor
WorldSession::~WorldSession()
{
	/// not login game
	if (!_player)
	{
		SendLoginError(3);
	}
    /// - If have unclosed socket, close it
    if (_Socket)
    {
        _Socket->CloseSocket();
        _Socket = nullptr;
    }

    ///- empty incoming packet queue
    WorldPacket* packet = NULL;
    while (_recvQueue.next(packet))
        delete packet;
}

char const * WorldSession::GetPlayerName() const
{
	return _player != NULL ? _player->GetName() : DefaultPlayerName;
}

std::string WorldSession::GetPlayerInfo() const
{
	std::ostringstream ss;

	ss << "[Player: " << GetPlayerName();
	ss<< ", Account: " << getAccountId() << ")]";

	return ss.str();
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket* packet)
{
    if (!_Socket)
        return;

#ifdef TRINITY_DEBUG
    // Code for network use statistic
    static uint64 sendPacketCount = 0;
    static uint64 sendPacketBytes = 0;

    static time_t firstTime = time(NULL);
    static time_t lastTime = firstTime;                     // next 60 secs start time

    static uint64 sendLastPacketCount = 0;
    static uint64 sendLastPacketBytes = 0;

    time_t cur_time = time(NULL);

    if ((cur_time - lastTime) < 60)
    {
        sendPacketCount+=1;
        sendPacketBytes+=packet->size();

        sendLastPacketCount+=1;
        sendLastPacketBytes+=packet->size();
    }
    else
    {
        uint64 minTime = uint64(cur_time - lastTime);
        uint64 fullTime = uint64(lastTime - firstTime);
        TC_LOG_INFO("misc", "Send all time packets count: " UI64FMTD " bytes: " UI64FMTD " avr.count/sec: %f avr.bytes/sec: %f time: %u", sendPacketCount, sendPacketBytes, float(sendPacketCount)/fullTime, float(sendPacketBytes)/fullTime, uint32(fullTime));
        TC_LOG_INFO("misc", "Send last min packets count: " UI64FMTD " bytes: " UI64FMTD " avr.count/sec: %f avr.bytes/sec: %f", sendLastPacketCount, sendLastPacketBytes, float(sendLastPacketCount)/minTime, float(sendLastPacketBytes)/minTime);

        lastTime = cur_time;
        sendLastPacketCount = 1;
        sendLastPacketBytes = packet->wpos();               // wpos is real written size
    }
#endif                                                      // !TRINITY_DEBUG

    _Socket->AsyncWrite(*packet);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

/// Update the WorldSession (triggered by World update)

bool WorldSession::Update(uint32 diff)
{   
	/// Update Timeout timer.
	UpdateTimeOutTime(diff);

	if (IsTimeOutTime())
		_Socket->CloseSocket();

    WorldPacket* packet = NULL;

    uint32 processedPackets = 0;

	while (_Socket && !_recvQueue.empty() && _recvQueue.next(packet))
    {
		OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];

		(this->*opHandle.handler)(*packet);

#define MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE 100
        processedPackets++;

        if (processedPackets > MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE)
            break;
    }
        ///- Cleanup socket pointer if need
        if (_Socket && !_Socket->IsOpen())
        {
             _Socket = nullptr;
        }

        if (!_Socket)
            return false;                                   
  
    return true;
}

void WorldSession::Handle_NULL(WorldPacket& recvPacket)
{
	TC_LOG_ERROR("network.opcode", "Received unhandled opcode %s from %s"
		, GetOpcodeNameForLogging(recvPacket.GetOpcode()).c_str(), GetPlayerInfo().c_str());
}

void WorldSession::SendLoginError(uint8 code)
{
	WorldPacket packet(CMSG_PLAYER_LOGIN, 4);
	packet << uint32(0);
	packet << uint32(0);
	packet << uint32(code);

	SendPacket(&packet);
}

void WorldSession::HandlePlayerLogin(WorldPacket& recvPacket)
{
	uint32 spaceid,roomid, SameRoom;
	PlayerInfo pInfo;

	recvPacket >>spaceid>> roomid >> SameRoom;
	recvPacket.read((uint8 *)&pInfo, sizeof(PlayerInfo));
	if (sRoomMgr->getPlayer(pInfo.id))
	{

	}
	else
	{
		_player = new Player(this);
		_player->loadData(pInfo);
		_player->setRoomId(roomid);
		sRoomMgr->AddPlayer(roomid, _player);

		WorldPacket packet(CMSG_PLAYER_LOGIN,600);

		packet << uint32(0) << uint32(0) << uint32(1);

		packet.resize(600);

		SendPacket(&packet);
	}
}

void WorldSession::HandleWaitStart(WorldPacket& recvPacket)
{
	getPlayer()->setStart();
}

void WorldSession::HandleGrabLandlord(WorldPacket& recvPacket)
{
	recvPacket >> getPlayer()->_grabLandlordScore;

	getPlayer()->setGameStatus(GAME_STATUS_GRAB_LAND_LORDING);
}