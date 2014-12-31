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
	if (_player != nullptr)
	{
		_player->logOutPlayer();
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

		delete packet;

#define MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE 100
        processedPackets++;

        if (processedPackets > MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE)
            break;
    }
        ///- Cleanup socket pointer if need
        if (_Socket && !_Socket->IsOpen() || getPlayer() == nullptr)
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

void WorldSession::KickPlayer()
{
	if (_Socket)
	{
		SendLoginError(LOGIN_HAVED_LOGIN);
		_Socket->CloseSocket();
	}
}

bool WorldSession::isRealPlayer()
{
	return _player->getPlayerType() == PLAYER_TYPE_USER;
}

void WorldSession::SendLoginError(int8 code)
{
	WorldPacket packet(CMSG_PLAYER_LOGIN, 4);

	packet << code;

	SendPacket(&packet);
}

void WorldSession::HandlePlayerLogin(WorldPacket& recvPacket)
{
	uint8 roomid;
	PlayerInfo pInfo;

	recvPacket >> roomid;
	recvPacket.read((uint8 *)&pInfo, sizeof(PlayerInfo));

	_player = sRoomMgr->getPlayer(pInfo.id);
	if (_player != nullptr)
	{
		_player->loadData(pInfo);
		_player->setSession(this);
		/// stop replace ai action
		_player->stopAiAction();
		recoverScene();
		_player->setPlayerType(PLAYER_TYPE_USER);
	}
	else
	{
		_player = new Player(this);
		_player->loadData(pInfo);
		_player->setRoomId(roomid);
		sRoomMgr->AddPlayer(roomid, _player);

		WorldPacket packet(CMSG_PLAYER_LOGIN,4);

		packet << LOGIN_SUCCESS;
		SendPacket(&packet);
	}
	TC_LOG_INFO("server.worldserver", "Player: %s login,remote IP: %s", GetPlayerInfo().c_str(), _Address.c_str());
}

void WorldSession::HandleWaitStart(WorldPacket& recvPacket)
{
	getPlayer()->setStart();
}

void WorldSession::HandleGrabLandlord(WorldPacket& recvPacket)
{
	recvPacket >> getPlayer()->_grabLandlordScore;

	getPlayer()->setGameStatus(GAME_STATUS_GRABING_LANDLORD);
}

void WorldSession::HandleOutCards(WorldPacket& recvPacket)
{
	Player * player = getPlayer();

	recvPacket.read((uint8 *)&player->_cardType,4);
	recvPacket.read((uint8 *)player->_outCards, 24);

	player->setGameStatus(GAME_STATUS_OUT_CARDING);
}

void WorldSession::HandleGetLeftPlayerCards(WorldPacket& recvPacket)
{
	Player * player = getPlayer();

	player->setGameStatus(GAME_STATUS_GETING_LEFTCARD);
}

void WorldSession::HandleRoundOver(WorldPacket& recvPacket)
{
	Player * player = getPlayer();

	recvPacket >> player->_winGold;

	player->setGameStatus(GAME_STATUS_ROUNDOVERING);
}

void WorldSession::HandlLogout(WorldPacket& recvPacket)
{
	Player * player = getPlayer();
	GameStatus preStatus = player->getGameStatus();

	player->setGameStatus(GameStatus(preStatus | GAME_STATUS_LOG_OUTING));
}

int8 WorldSession::getCurGameStatus()
{
	Player * left = _player->_left;
	Player * right = _player->_right;
	if (_player->getGameStatus() < GAME_STATUS_START_OUT_CARD
		&& left->getGameStatus() < GAME_STATUS_START_OUT_CARD
		&& right->getGameStatus() < GAME_STATUS_START_OUT_CARD)
	{
		return LOGIN_GRANING_LANDLORD;
	}
	else if (_player->getGameStatus() < GAME_STATUS_ROUNDOVERING)
	{
		return LOGIN_OUT_CARDING;
	}
	else
	{
		return LOGIN_GAMEOVER;
	}
}

void WorldSession::recoverScene()
{
	int8 curGameStatus = getCurGameStatus();
	Player * left = _player->_left;
	Player * right = _player->_right;

	if (curGameStatus == LOGIN_GRANING_LANDLORD)
	{
		WorldPacket packet(CMSG_PLAYER_LOGIN, 20);

		packet << uint8(LOGIN_GRANING_LANDLORD);

		packet.append((uint8 *)left->getPlayerInfo(), sizeof(PlayerInfo));
		packet.append((uint8 *)right->getPlayerInfo(), sizeof(PlayerInfo));

		packet << _player->getDefaultLandlordUserId();
		packet << left->getGrabLandlordScore();
		packet << right->getGrabLandlordScore();
		packet << _player->getGrabLandlordScore();

		SendPacket(&packet);
	}
	else if (curGameStatus == LOGIN_OUT_CARDING)
	{
		WorldPacket packet(CMSG_PLAYER_LOGIN, 459);//1 + 152 * 2 + 4 + 3 + 20 + 4 + 20 * 3 + 20 * 3 + 1 * 3

		packet << uint8(LOGIN_OUT_CARDING);

		packet.append((uint8 *)left->getPlayerInfo(), sizeof(PlayerInfo));
		packet.append((uint8 *)right->getPlayerInfo(), sizeof(PlayerInfo));

		int32 LandlordScore = std::max(_player->getGrabLandlordScore(),std::max(left->getGrabLandlordScore(),right->getGrabLandlordScore()));
		packet << LandlordScore;
		packet << _player->getLandlordId();

		packet.append(_player->_baseCards, 3);

		packet.append(_player->_cards, 20);

		//packet << _player->_curOutCardsPlayer->getid();
		DeskFlag outingCardsPlayerDeskFlag = _player->getGameStatus() == GAME_STATUS_START_OUT_CARD ? SELF :
			(left->getGameStatus() == GAME_STATUS_START_OUT_CARD ? LEFT : RIGHT);

		packet << (uint8)outingCardsPlayerDeskFlag;
		packet << (uint8)_player->_curOutCardType;

		packet.append(left->_selfAllOutCards, 20);
		packet.append(right->_selfAllOutCards, 20);
		packet.append(_player->_selfAllOutCards, 20);

		packet.append(left->_outCards, 20);
		packet.append(right->_outCards, 20);
		packet.append(_player->_outCards, 20);

		packet << left->_bombCount;
		packet << right->_bombCount;
		packet << _player->_bombCount;

		SendPacket(&packet);
	}
	else
	{
		_player->handleRoundOver();
	}
}