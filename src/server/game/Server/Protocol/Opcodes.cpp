
#include "Opcodes.h"
#include "WorldSession.h"

/// Correspondence between opcodes and their names
OpcodeHandler opcodeTable[NUM_MSG_TYPES] =
{
	/*0x000*/{ "MSG_NULL_ACTION", &WorldSession::Handle_NULL },
};
