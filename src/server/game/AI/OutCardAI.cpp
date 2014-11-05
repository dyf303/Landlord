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
	if (player->getLandlordId() == player->getid() && getCardsNumber(player->_cards) == 20
		|| (player->_left->_cardType == CARD_TYPE_PASS && player->_right->_cardType == CARD_TYPE_PASS))
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

void OutCardAi::updateCardsFace(uint8* pcSelfCard, uint8* pcOutCard)
{
	int iCardIdx = 0;
	if (*pcOutCard == CARD_TERMINATE)
	{
		return ;
	}
	while (pcOutCard[iCardIdx] != CARD_TERMINATE)
	{
		char cTmpCard = pcOutCard[iCardIdx];
		rearrangeCards(pcSelfCard, cTmpCard);
		iCardIdx++;
	}
}

void OutCardAi::rearrangeCards(uint8* SelfCards, uint8 cCard)
{
	int iCardIdx = 0;
	for (; SelfCards[iCardIdx] != CARD_TERMINATE; iCardIdx++)
	{
		if (SelfCards[iCardIdx] == cCard)
		{
			break;
		}
	}
	for (; SelfCards[iCardIdx] != CARD_TERMINATE; iCardIdx++)
	{
		SelfCards[iCardIdx] = SelfCards[iCardIdx + 1];
	}
}
