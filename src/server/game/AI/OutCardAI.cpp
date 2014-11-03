#include "OutCardAI.h"

#include "Player.h"
OutCardAi::OutCardAi()
{
}

OutCardAi::~OutCardAi()
{
}

uint32 OutCardAi::getCardsNumber(uint8 * cards)
{
	uint32 number = 0;
	while (cards[number] != CARD_TERMINATE)
		++number;
	return number;
}

void OutCardAi::resetCards(uint8 * cards)
{
	for (uint8 i = 0; i < 24; ++i)
	{
		cards[i] = CARD_TERMINATE;
	}
}

void OutCardAi::OutCard(Player *player)
{
	/// test
	if (player->getLandlordId() == player->getid() && getCardsNumber(player->_cards) == 20)
	{
		resetCards(player->_outCards);
		player->_outCards[0] = player->_cards[0];
		player->_cardType = CARD_TYPE_SINGLE;
	}
	else
	{
		resetCards(player->_outCards);
		player->_cardType = CARD_TYPE_PASS;
	}
}