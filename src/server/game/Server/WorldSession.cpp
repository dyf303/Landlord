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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"


//we should process ALL packets when player is not in world/logged in
//OR packet handler is not thread-safe!
bool WorldSessionFilter::Process(WorldPacket* packet)
{
    OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];
    //check if packet handler is supposed to be safe
    if (opHandle.packetProcessing == PROCESS_INPLACE)
        return true;

    //thread-unsafe packets should be processed in World::UpdateSessions()
    if (opHandle.packetProcessing == PROCESS_THREADUNSAFE)
        return true;

    //no player attached? -> our client! ^^
    //Player* player = m_pSession->GetPlayer();
    //if (!player)
    //    return true;

    //lets process all packets for non-in-the-world player
 //   return (player->IsInWorld() == false);
}

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, std::shared_ptr<WorldSocket> sock):
    m_Socket(sock),
    _accountId(id),
	forceExit(false)
{
    if (sock)
    {
        m_Address = sock->GetRemoteIpAddress().to_string();
    }


}

/// WorldSession destructor
WorldSession::~WorldSession()
{
    /// - If have unclosed socket, close it
    if (m_Socket)
    {
        m_Socket->CloseSocket();
        m_Socket = nullptr;
    }

    ///- empty incoming packet queue
    WorldPacket* packet = NULL;
    while (_recvQueue.next(packet))
        delete packet;
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket* packet)
{
    if (!m_Socket)
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

    m_Socket->AsyncWrite(*packet);
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    _recvQueue.add(new_packet);
}

/// Update the WorldSession (triggered by World update)
/*
bool WorldSession::Update(uint32 diff, PacketFilter& updater)
{
    ///- Before we process anything:
    /// If necessary, kick the player from the character select screen
    if (IsConnectionIdle())
        m_Socket->CloseSocket();

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not process packets if socket already closed
    WorldPacket* packet = NULL;
    //! Delete packet after processing by default
    bool deletePacket = true;
    //! To prevent infinite loop
    WorldPacket* firstDelayedPacket = NULL;
    //! If _recvQueue.peek() == firstDelayedPacket it means that in this Update call, we've processed all
    //! *properly timed* packets, and we're now at the part of the queue where we find
    //! delayed packets that were re-enqueued due to improper timing. To prevent an infinite
    //! loop caused by re-enqueueing the same packets over and over again, we stop updating this session
    //! and continue updating others. The re-enqueued packets will be handled in the next Update call for this session.
    uint32 processedPackets = 0;
    time_t currentTime = time(NULL);

    while (m_Socket && !_recvQueue.empty() && _recvQueue.peek(true) != firstDelayedPacket && _recvQueue.next(packet, updater))
    {
        if (!AntiDOS.EvaluateOpcode(*packet, currentTime))
        {
            KickPlayer();
        }
        else if (packet->GetOpcode() >= NUM_MSG_TYPES)
        {
            TC_LOG_ERROR("network.opcode", "Received non-existed opcode %s from %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str()
                            , GetPlayerInfo().c_str());
            sScriptMgr->OnUnknownPacketReceive(this, *packet);
        }
        else
        {
            OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];
            try
            {
                switch (opHandle.status)
                {
                    case STATUS_LOGGEDIN:
                        if (!_player)
                        {
                            // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
                            //! If player didn't log out a while ago, it means packets are being sent while the server does not recognize
                            //! the client to be in world yet. We will re-add the packets to the bottom of the queue and process them later.
                            if (!m_playerRecentlyLogout)
                            {
                                //! Prevent infinite loop
                                if (!firstDelayedPacket)
                                    firstDelayedPacket = packet;
                                //! Because checking a bool is faster than reallocating memory
                                deletePacket = false;
                                QueuePacket(packet);
                                //! Log
                                TC_LOG_DEBUG("network", "Re-enqueueing packet with opcode %s with with status STATUS_LOGGEDIN. "
                                    "Player is currently not in world yet.", GetOpcodeNameForLogging(packet->GetOpcode()).c_str());
                            }
                        }
                        else if (_player->IsInWorld())
                        {
                            sScriptMgr->OnPacketReceive(this, *packet);
                            (this->*opHandle.handler)(*packet);
                            LogUnprocessedTail(packet);
                        }
                        // lag can cause STATUS_LOGGEDIN opcodes to arrive after the player started a transfer
                        break;
                    case STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT:
                        if (!_player && !m_playerRecentlyLogout && !m_playerLogout) // There's a short delay between _player = null and m_playerRecentlyLogout = true during logout
                            LogUnexpectedOpcode(packet, "STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT",
                                "the player has not logged in yet and not recently logout");
                        else
                        {
                            // not expected _player or must checked in packet handler
                            sScriptMgr->OnPacketReceive(this, *packet);
                            (this->*opHandle.handler)(*packet);
                            LogUnprocessedTail(packet);
                        }
                        break;
                    case STATUS_TRANSFER:
                        if (!_player)
                            LogUnexpectedOpcode(packet, "STATUS_TRANSFER", "the player has not logged in yet");
                        else if (_player->IsInWorld())
                            LogUnexpectedOpcode(packet, "STATUS_TRANSFER", "the player is still in world");
                        else
                        {
                            sScriptMgr->OnPacketReceive(this, *packet);
                            (this->*opHandle.handler)(*packet);
                            LogUnprocessedTail(packet);
                        }
                        break;
                    case STATUS_AUTHED:
                        // prevent cheating with skip queue wait
                        if (m_inQueue)
                        {
                            LogUnexpectedOpcode(packet, "STATUS_AUTHED", "the player not pass queue yet");
                            break;
                        }

                        // some auth opcodes can be recieved before STATUS_LOGGEDIN_OR_RECENTLY_LOGGOUT opcodes
                        // however when we recieve CMSG_CHAR_ENUM we are surely no longer during the logout process.
                        if (packet->GetOpcode() == CMSG_CHAR_ENUM)
                            m_playerRecentlyLogout = false;

                        sScriptMgr->OnPacketReceive(this, *packet);
                        (this->*opHandle.handler)(*packet);
                        LogUnprocessedTail(packet);
                        break;
                    case STATUS_NEVER:
                        TC_LOG_ERROR("network.opcode", "Received not allowed opcode %s from %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str()
                            , GetPlayerInfo().c_str());
                        break;
                    case STATUS_UNHANDLED:
                        TC_LOG_DEBUG("network.opcode", "Received not handled opcode %s from %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str()
                            , GetPlayerInfo().c_str());
                        break;
                }
            }
            catch (ByteBufferException const&)
            {
                TC_LOG_ERROR("misc", "WorldSession::Update ByteBufferException occured while parsing a packet (opcode: %u) from client %s, accountid=%i. Skipped packet.",
                        packet->GetOpcode(), GetRemoteAddress().c_str(), GetAccountId());
                packet->hexlike();
            }
        }

        if (deletePacket)
            delete packet;

        deletePacket = true;

#define MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE 100
        processedPackets++;

        //process only a max amout of packets in 1 Update() call.
        //Any leftover will be processed in next update
        if (processedPackets > MAX_PROCESSED_PACKETS_IN_SAME_WORLDSESSION_UPDATE)
            break;
    }

    if (m_Socket && m_Socket->IsOpen() && _warden)
        _warden->Update();

    ProcessQueryCallbacks();

    //check if we are safe to proceed with logout
    //logout procedure should happen only in World::UpdateSessions() method!!!
    if (updater.ProcessLogout())
    {
        time_t currTime = time(NULL);
        ///- If necessary, log the player out
        if (ShouldLogOut(currTime) && !m_playerLoading)
            LogoutPlayer(true);

        if (m_Socket && GetPlayer() && _warden)
            _warden->Update();

        ///- Cleanup socket pointer if need
        if (m_Socket && !m_Socket->IsOpen())
        {
            expireTime -= expireTime > diff ? diff : expireTime;
            if (expireTime < diff || forceExit || !GetPlayer())
            {
                m_Socket = nullptr;
            }
        }

        if (!m_Socket)
            return false;                                       //Will remove this session from the world session map
    }

    return true;
}

*/

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
	if (m_Socket)
	{
		m_Socket->CloseSocket();
		forceExit = true;
	}
}












































