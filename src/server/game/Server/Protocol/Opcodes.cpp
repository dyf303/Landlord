
#include "Opcodes.h"
#include "WorldSession.h"

/// Correspondence between opcodes and their names
OpcodeHandler opcodeTable[NUM_MSG_TYPES] =
{
	/*0x00*/{ "CMSG_PLAYER_LOGIN",               &WorldSession::HandlePlayerLogin },
	/*0x01*/{ "SMSG_DESK_TWO",                   &WorldSession::Handle_NULL },
	/*0x02*/{ "SMSG_DESK_THREE",                 &WorldSession::Handle_NULL },
	/*0x03*/{ "CMSG_WAIT_START",                 &WorldSession::HandleWaitStart },
	/*0x04*/{ "SMSG_CARD_DEAL",                  &WorldSession::Handle_NULL },
	/*0x05*/{ "CMSG_GRAD_LANDLORD",              &WorldSession::HandleGrabLandlord },
	/*0x06*/{ "CMSG_DOUBLE_SCORE",               &WorldSession::Handle_NULL },
	/*0x07*/{ "CMSG_SHOW_CARD",                  &WorldSession::Handle_NULL },
	/*0x08*/{ "CMSG_CARD_OUT",                   &WorldSession::HandleOutCards },
	/*0x09*/{ "CMSG_REQUEST_CARDS_LEFT",         &WorldSession::Handle_NULL },
	/*0x0A*/{ "CMSG_ROUND_OVER",                 &WorldSession::HandleRoundOver },
	/*0x0B*/{ "CMSG_CHANGE_DESK",                &WorldSession::Handle_NULL },
	/*0x0C*/{ "CMSG_CHAT_SHORTCUT",              &WorldSession::Handle_NULL },
	/*0x0D*/{ "CMSG_CHAT_ICON",                  &WorldSession::Handle_NULL },
	/*0x0E*/{ "CMSG_CHAT_CONTEXT",               &WorldSession::Handle_NULL },
	/*0x0F*/{ "CMSG_PING",                       &WorldSession::Handle_NULL },
	/*0x10*/{ "CMSG_EXIT",                       &WorldSession::Handle_NULL },
	/*0x11*/{ "CMSG_INCREMENT_GOLD",             &WorldSession::Handle_NULL },
};
