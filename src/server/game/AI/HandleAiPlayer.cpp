#include "Player.h"

#include "OutCardAI.h"
#include "WorldPacket.h"
#include "World.h"

void Player::UpdateAiDelay(uint32 diff)
{
	if (_aiGameStatus == AI_GAME_STATUS_GRAD_LANDLORD
		|| _aiGameStatus == AI_GAME_STATUS_OUT_CARD)
		_aiDelay -= diff;

	if (_aiDelay < 0)
	{
		switch (_aiGameStatus)
		{
		case AI_GAME_STATUS_NULL:break;
		case AI_GAME_STATUS_GRAD_LANDLORD:_gameStatus = GAME_STATUS_GRABING_LANDLORD; break;
		case AI_GAME_STATUS_OUT_CARD:_gameStatus = GAME_STATUS_OUT_CARDING; break;
		default:break;
		}
		_aiGameStatus = AI_GAME_STATUS_NULL;
	}
}

uint32 Player::aiGrabLandlord(uint32 score)
{
	uint32 aiScore = 0;
	switch (score)
	{
	case 0:aiScore = rand() % 4; break;
	case 1:aiScore = 0; break;
	case 2:aiScore = 0; break;
	default:break;
	}
	return aiScore;
}

void Player::aiRecvPacket(WorldPacket* packet)
{
	uint32 opcode = packet->GetOpcode();

	switch (opcode)
	{
	case SMSG_CARD_DEAL:aiHandleDealCard(packet); break;
	case CMSG_GRAD_LANDLORD:aiHandleGrabLandlord(packet); break;
	case CMSG_CARD_OUT:aiHandleOutCards(packet); break;
	case CMSG_LOG_OUT:aiHandlLogout(packet); break;
	default:break;
	}
	packet->clearRead();
}

void Player::aiHandleDealCard(WorldPacket* packet)
{
	if (getDefaultLandlordUserId() == getid())
	{
		_grabLandlordScore = aiGrabLandlord(0);
		_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY) * 2;
		_aiGameStatus = AI_GAME_STATUS_GRAD_LANDLORD;
	}
}

void Player::aiHandleGrabLandlord(WorldPacket* packet)
{
	uint32 grabLandlordPlayerId;
	uint32 grabLandlordScore;
	uint32 landlordId;

	packet->read_skip(8);
	*packet >> grabLandlordPlayerId;
	*packet >> grabLandlordScore;
	*packet >> landlordId;

	if (getLandlordId() == getid())
	{
		sOutCardAi->OutCard(this);
		_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
		_aiGameStatus = AI_GAME_STATUS_OUT_CARD;
	}

	if (grabLandlordPlayerId == _left->getid() && landlordId == -1)
	{
		_grabLandlordScore = aiGrabLandlord(grabLandlordScore);
		_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY) * 2;
		_aiGameStatus = AI_GAME_STATUS_GRAD_LANDLORD;
	}
}

void Player::aiHandleOutCards(WorldPacket* packet)
{
	uint32 outCardsPlayerId;
	uint32 outCardsType;
	uint8  outCards[24];

	packet->read_skip(8);
	*packet >> outCardsPlayerId;
	*packet >> outCardsType;
	packet->read((uint8 *)outCards, 24);

	if (RoundOver(outCardsPlayerId))
	{
		_gameStatus = GAME_STATUS_ROUNDOVERING;
		return;
	}

	if (outCardsPlayerId == _left->getid())
	{
		sOutCardAi->OutCard(this);
		_aiDelay = sWorld->getIntConfig(CONFIG_AI_DELAY);
		_aiGameStatus = AI_GAME_STATUS_OUT_CARD;
	}
}

bool Player::RoundOver(uint32 outCardPlayerId)
{
	Player *outCardPlayer = (getid() == outCardPlayerId ? this : (_left->getid() == outCardPlayerId ? _left : _right));

	return 0 == sOutCardAi->getCardsNumber(outCardPlayer->_cards);
}

void Player::aiHandlLogout(WorldPacket* packet)
{
	uint32 logoutPlayerId;
	
	packet->read_skip(8);
	*packet >> logoutPlayerId;

	if (_left == nullptr || _right == nullptr
		|| (_left->getPlayerType()&PLAYER_TYPE_AI && _right->getPlayerType()&PLAYER_TYPE_AI))
	{
		_gameStatus = GAME_STATUS_LOG_OUTED;
	}
}