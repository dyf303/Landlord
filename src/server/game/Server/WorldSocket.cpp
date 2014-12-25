
#include "WorldSocket.h"
#include "Opcodes.h"
#include "PacketLog.h"
#include "Player.h"
#include <memory>

using boost::asio::ip::tcp;

WorldSocket::WorldSocket(tcp::socket&& socket)
    : Socket(std::move(socket), sizeof(ClientPktHeader)), _worldSession(nullptr)
{
}

void WorldSocket::Start()
{
    AsyncReadHeader();
}

void WorldSocket::ReadHeaderHandler()
{
    ClientPktHeader* header = reinterpret_cast<ClientPktHeader*>(GetHeaderBuffer());

    if (!header->IsValid())
    {
        if (_worldSession)
        {
            //Player* player = _worldSession->getPlayer();
           // TC_LOG_ERROR("network", "WorldSocket::ReadHeaderHandler(): client (account: %u, char [GUID: %u, name: %s]) sent malformed packet (size: %hu, cmd: %u)",
               // _worldSession->GetAccountId(), player ? player->GetGUIDLow() : 0, player ? player->GetName().c_str() : "<none>", header->size, header->cmd);
        }
        else
            TC_LOG_ERROR("network", "WorldSocket::ReadHeaderHandler(): client %s sent malformed packet (size: %hu, cmd: %u)",
                GetRemoteIpAddress().to_string().c_str(), header->size, header->cmd);

        CloseSocket();
        return;
    }

	AsyncReadData(header->size);
}

void WorldSocket::ReadDataHandler()
{
    ClientPktHeader* header = reinterpret_cast<ClientPktHeader*>(GetHeaderBuffer());

    uint8 opcode = uint16(header->cmd);

    std::string opcodeName = GetOpcodeNameForLogging(opcode);

    WorldPacket packet(opcode, MoveData());

    if (sPacketLog->CanLogPacket())
        sPacketLog->LogPacket(packet, CLIENT_TO_SERVER, GetRemoteIpAddress(), GetRemotePort());

    TC_LOG_TRACE("network.opcode", "C->S: %s %s", (_worldSession ? _worldSession->GetPlayerInfo() : GetRemoteIpAddress().to_string()).c_str(), opcodeName.c_str());
	
    switch (opcode)
    {
		case CMSG_PLAYER_LOGIN:
            if (_worldSession)
            {
                TC_LOG_ERROR("network", "WorldSocket::ProcessIncoming: received duplicate CMSG_AUTH_SESSION from %s", _worldSession->GetPlayerInfo().c_str());
                break;
            }

            AddSession(packet);
            break;
		case CMSG_PING:_worldSession->ResetTimeOutTime(); break;

        default:
        {
            if (!_worldSession)
            {
                TC_LOG_ERROR("network.opcode", "ProcessIncoming: Client not authed opcode = %u", uint32(opcode));
                CloseSocket();
                return;
            }
            _worldSession->ResetTimeOutTime();

            // Copy the packet to the heap before enqueuing
            _worldSession->QueuePacket(new WorldPacket(std::move(packet)));
            break;
        }
    }

    AsyncReadHeader();
}

void WorldSocket::AsyncWrite(WorldPacket& packet)
{
    if (!IsOpen())
        return;

    if (sPacketLog->CanLogPacket())
        sPacketLog->LogPacket(packet, SERVER_TO_CLIENT, GetRemoteIpAddress(), GetRemotePort());

    TC_LOG_TRACE("network.opcode", "S->C: %s %s", (_worldSession ? _worldSession->GetPlayerInfo() : GetRemoteIpAddress().to_string()).c_str(), GetOpcodeNameForLogging(packet.GetOpcode()).c_str());

	uint8 Opcode = packet.GetOpcode();
	ServerPktHeader header(packet.size(), Opcode);

	std::lock_guard<std::mutex> guard(_writeLock);

    bool needsWriteStart = _writeQueue.empty();

    _writeQueue.emplace(header, packet);

    if (needsWriteStart)
        AsyncWrite(_writeQueue.front());
}

void WorldSocket::AddSession(WorldPacket& recvPacket)
{
	_worldSession = new WorldSession(recvPacket.peek<uint32>(1), shared_from_this());
	_worldSession->QueuePacket(new WorldPacket(std::move(recvPacket)));
	_worldSession->ResetTimeOutTime();
	sWorld->AddSession(_worldSession);
}

void WorldSocket::CloseSocket()
{
    Socket::CloseSocket();
}
