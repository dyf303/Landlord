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

#ifndef _OPCODES_H
#define _OPCODES_H

#include "Common.h"

/// List of Opcodes
enum Opcodes
{
	MSG_NULL_ACTION                 = 0x00,
	CMSG__CONNECT                   = 0x00,	                          /// 0���ӷ��������
	SMSG_DESK_TWO                   = 0x01,							  /// 1˫����
	CMSG_DESK_THREE                 = 0x02,							  /// 2������
	CMSG_WAIT_START                 = 0x03,							  /// 3�ȴ���ʼ
	CMSG_CARD_DEAL                  = 0x04,							  /// 4����
	CMSG_GRAD_LANDLORD              = 0X05,						      /// 5�з�
	CMSG_DOUBLE_SCORE               = 0x06,						      /// 6�ӱ�
	CMSG_SHOW_CARD                  = 0x07,						      /// 7����
	CMSG_CARD_OUT                   = 0x08,						      /// 8����
	CMSG_REQUEST_CARDS_LEFT         = 0x09 ,                          /// 9�����������������������Ϣ
	CMSG_ROUND_OVER                 = 0x0A    ,                       /// 10һ�ֽ���(����Ҫ�ϴ�����)
	CMSG_CHANGE_DESK                = 0x0B  ,                         /// 11������
	CMSG_CHAT_SHORTCUT              = 0x0C,						      /// 12�����
	CMSG_CHAT_ICON                  = 0x0D,							  /// 13����
	CMSG_CHAT_CONTEXT               = 0x0E,						      /// 14������������
	CMSG_PING                       = 0x0F ,                          /// 15��������������
	CMSG_EXIT                       = 0x10,						      /// 16�˳�����
	CMSG_INCREMENT_GOLD             = 0x11   ,                        /// 17�������ӽ�ҷ���
    NUM_MSG_TYPES                   = 0x12
};


class WorldSession;
class WorldPacket;

#if defined(__GNUC__)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

struct OpcodeHandler
{
    char const* name;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

extern OpcodeHandler opcodeTable[NUM_MSG_TYPES];

#if defined(__GNUC__)
#pragma pack()
#else
#pragma pack(pop)
#endif

/// Lookup opcode name for human understandable logging
inline const char* LookupOpcodeName(uint16 id)
{
    if (id >= NUM_MSG_TYPES)
        return "Received unknown opcode, it's more than max!";
    return opcodeTable[id].name;
}

inline std::string GetOpcodeNameForLogging(uint16 opcode)
{
    std::ostringstream ss;
    ss << '[' << LookupOpcodeName(opcode) << " 0x" << std::hex << std::uppercase << opcode << std::nouppercase << " (" << std::dec << opcode << ")]";
    return ss.str();
}

#endif
/// @}
