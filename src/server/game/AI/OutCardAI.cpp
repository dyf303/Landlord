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

void OutCardAi::updateCardsFace(uint8* SelfCards, uint8* OutCards)
{
	int32 iCardIdx = 0;
	if (*OutCards == CARD_TERMINATE)
	{
		return ;
	}
	while (OutCards[iCardIdx] != CARD_TERMINATE)
	{
		uint8 cTmpCard = OutCards[iCardIdx];
		rearrangeCards(SelfCards, cTmpCard);
		iCardIdx++;
	}
}

void OutCardAi::rearrangeCards(uint8* SelfCards, uint8 cCard)
{
	int32 iCardIdx = 0;
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

CardType OutCardAi::getPlayOutCardFirst(Player *player)
{
	/// 如果我是地主
	if (player->getLandlordId() == player->getid())
	{
		/// 直接出牌
		return getPlayOutCardFirst(player->_cards, player->_outCards);
	}

	/// 自己手里是不是一手牌(可以先走)
	if (-1 != IsSeriesCard(player->_cards, player->_outCards))
	{
		/// 直接出牌
	//	memset(pkQueueDeskDataNode->pcCardsOutAi, CARD_TERMINATE, sizeof(pkQueueDeskDataNode->pcCardsOutAi));
		return getPlayOutCardFirst(player->_cards, player->_outCards);
	}

	/// 如果下家是对家，获得对家手中的剩余牌数(小心别送走对家)
	if (player->getPlayerGameType() != player->_right->getPlayerGameType())
	{
		/// 获得对家手中剩余牌数
		int32 CardsEnemyNum = getCardsNumber(player->_right->_cards);

		/// 小于5张，出牌要小心(别把对家给送走了)
		if (5 > CardsEnemyNum)
		{
			/// 获得自己手中剩余牌数
			int32 CardsSelfNum = getCardsNumber(player->_cards);

			/// 对家手中只有1张
			if (1 == CardsEnemyNum)
			{
				/// 出单支以外的牌
				if (1 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_SINGLE, CardsSelfNum);
				}
			}
			else if (2 == CardsEnemyNum) /// 等于2张
			{
				/// 出对子以外的牌
				if (2 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_PAIR, CardsSelfNum);
				}
			}
			else if (3 == CardsEnemyNum) /// 等于3张
			{
				/// 出三支以外的牌
				if (3 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_TRPILE, CardsSelfNum);
				}
			}
			else if (4 == CardsEnemyNum) /// 等于4张
			{
				/// 出三带一以外的牌
				if (4 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_TRIPLE_ONE, CardsSelfNum);
				}
			}
		}
	}

	/// 获得我对方自家手中剩余的牌数(方便送走自家)
	/// 获得自家在桌子上的位置
	Player *their = (player->getPlayerGameType() == player->_left->getPlayerGameType ? player->_left : player->_right);

	/// 获得对方自家手中剩余牌数
	int32 iCardsSameFamilyNum = getCardsNumber(their->_cards);

	/// 大于等于5张，直接出牌
	/// 小于5张，可以进行送牌
	if (5 > iCardsSameFamilyNum)
	{
		/// 等于1张
		if (1 == iCardsSameFamilyNum)
		{
			/// 出单支
			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
		else if (2 == iCardsSameFamilyNum) /// 等于2张
		{
			/// 出对子，没有对子出单支
			if (-1 != getPairCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_PAIR;
			}

			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
		else if (3 == iCardsSameFamilyNum) /// 等于3张
		{
			/// 出三支，没有三支出对子，没有对子出单支
			if (-1 != getTripleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_TRPILE;
			}

			if (-1 != getPairCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_PAIR;
			}

			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
		else if (4 == iCardsSameFamilyNum) /// 等于4张
		{
			/// 出三带一，没有三支，出对子，没有对子出单支
			if (-1 != getTripleWithOne(player->_cards, player->_outCards, 0, 4))
			{
				return CARD_TYPE_TRIPLE_ONE;
			}

			if (-1 != getTripleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_TRPILE;
			}

			if (-1 != getPairCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_PAIR;
			}

			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
	}

	return getPlayOutCardFirst(player->_cards, player->_outCards);
}

CardType OutCardAi::getPlayOutCardFirst(uint8 SelfCards[], uint8 OutCards[])
{
	CardType cardType = 0;

	int CardsNum = 0;
	while (SelfCards[CardsNum] != CARD_TERMINATE)
	{
		CardsNum++;
	}

	//if(CardsNum <= 10)
	{
		if (CardsNum == 10)
		{
			if (getAirPlane(SelfCards, OutCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(SelfCards, OutCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(SelfCards, OutCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 9)
		{
			if (getTripleProgression(SelfCards, OutCards, 0, 9) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getSingleProgression(SelfCards, OutCards, 0, 9) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 8)
		{
			if (getAirPlane(SelfCards, OutCards, 0, 8) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(SelfCards, OutCards, 0, 8) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(SelfCards, OutCards, 0, 8) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 6)
		{
			if (getTripleProgression(SelfCards, OutCards, 0, 6) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getPairProgression(SelfCards, OutCards, 0, 6) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(SelfCards, OutCards, 0, 6) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 5)
		{
			if (getSingleProgression(SelfCards, OutCards, 0, 5) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}

			if (getTripleWithTwo(SelfCards, OutCards, 0, 5) >= 0)
			{
				return CARD_TYPE_TRIPLE_TWO;
			}

		}
		if (CardsNum >= 4)
		{
			if (getTripleWithOne(SelfCards, OutCards, 0, 4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
		}
	}
	if ((SelfCards[0] & 0x0f) == 13 && (SelfCards[1] & 0x0f) == 14 && SelfCards[2] == CARD_TERMINATE)
	{
		OutCards[0] = SelfCards[0];
		OutCards[1] = SelfCards[1];
		return CARD_TYPE_ROCKET;
	}

	int iIdx = 0;

	while (SelfCards[iIdx] != CARD_TERMINATE)
	{
		if (SelfCards[iIdx + 1] == CARD_TERMINATE)
		{
			OutCards[0] = SelfCards[iIdx];
			cardType = CARD_TYPE_SINGLE;
			break;
		}
		else if ((SelfCards[iIdx] & 0x0f) != (SelfCards[iIdx + 1] & 0x0f))
		{
			OutCards[0] = SelfCards[iIdx];
			cardType = CARD_TYPE_SINGLE;
			break;
		}
		else if (SelfCards[iIdx + 2] == CARD_TERMINATE)
		{
			OutCards[0] = SelfCards[iIdx];
			OutCards[1] = SelfCards[iIdx + 1];
			cardType = CARD_TYPE_PAIR;
			break;
		}
		else if ((SelfCards[iIdx] & 0x0f) != (SelfCards[iIdx + 2] & 0x0f))
		{
			OutCards[0] = SelfCards[iIdx];
			OutCards[1] = SelfCards[iIdx + 1];
			cardType = CARD_TYPE_PAIR;
			break;
		}
		else if (SelfCards[iIdx + 3] == CARD_TERMINATE)
		{
			OutCards[0] = SelfCards[iIdx];
			OutCards[1] = SelfCards[iIdx + 1];
			OutCards[2] = SelfCards[iIdx + 2];
			cardType = CARD_TYPE_TRPILE;
			break;
		}
		else if ((SelfCards[iIdx] & 0x0f) != (SelfCards[iIdx + 3] & 0x0f))
		{
			OutCards[0] = SelfCards[iIdx];
			OutCards[1] = SelfCards[iIdx + 1];
			OutCards[2] = SelfCards[iIdx + 2];
			int iResult = getSingleCard(SelfCards, OutCards + 3, 0);
			if (iResult >= 0)
			{
				cardType = CARD_TYPE_TRIPLE_ONE;
				break;
			}
			else
			{
				iResult = getPairCard(SelfCards, OutCards + 3, 0);
				if (iResult >= 0)
				{
					CardType = CARD_TYPE_TRIPLE_TWO;

					break;
				}
				else
				{
					CardType = CARD_TYPE_TRPILE;
					break;
				}
			}
		}
		else
		{
			iIdx += 4;
		}
	}
	if (SelfCards[iIdx] != CARD_TERMINATE)
	{
		return cardType;
	}
	else
	{
		OutCards[0] = SelfCards[0];
		OutCards[1] = SelfCards[1];
		OutCards[2] = SelfCards[2];
		OutCards[3] = SelfCards[3];
		return CARD_TYPE_BOMB;
	}
}

CardType OutCardAi::IsSeriesCard(uint8 SelfCards[], uint8 OutCards[])
{
	uint32 SelfCardsNum = 0;
	while (SelfCards[SelfCardsNum] != CARD_TERMINATE)
	{
		SelfCardsNum++;
	}

	if (SelfCardsNum <= 10)
	{
		if (SelfCardsNum == 10 || SelfCardsNum == 8)
		{
			if (getAirPlane(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
		}
		if (iSelfCardsNum == 9)
		{
			if (getTripleProgression(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
		}
		if (iSelfCardsNum == 6)
		{
			if (getTripleProgression(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getPairProgression(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
		}
		if (iSelfCardsNum >= 5)
		{
			if (getSingleProgression(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
			if (iSelfCardsNum == 5)
			{
				if (getTripleWithTwo(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
				{
					return CARD_TYPE_TRIPLE_TWO;
				}
			}
		}
		if (iSelfCardsNum == 4)
		{
			if (getTripleWithOne(SelfCards, OutCards, 0, iSelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
		}
	}
	return -1;
}

CardType OutCardAi::playOutCardsNotByType(uint8 SelfCards[], uint8 OutCards[], int32 CardType, int32 SelfCardsNum)
{
	switch(CardType)
	{
	case CARD_TYPE_SINGLE:
		{
			if(getTripleWithOne(SelfCards,OutCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getTripleCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else if(getPairCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				OutCards[0]= SelfCards[SelfCardsNum -1];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_PAIR:
		{
			if(getTripleWithOne(SelfCards,OutCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getTripleCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else
			{
				OutCards[0]= SelfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_TRPILE:
		{
			if(getTripleWithOne(SelfCards,OutCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getPairCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				OutCards[0]= SelfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_TRIPLE_ONE:
		{
			if(getTripleCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else if(getPairCard(SelfCards,OutCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				OutCards[0]= SelfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	}

	return CARD_TYPE_PASS;
}

/*
 * getCard
 */
int32 OutCardAi::getSingleCard(uint8* SelfCards, uint8* cOutCard, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;
	while (SelfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{//0,1,2,3
		if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f))
		{
			break;
		}
		else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 2] & 0x0f))
		{
			iSelfIdx += 2;
		}
		else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 3] & 0x0f))
		{
			iSelfIdx += 3;
		}
		else
		{
			iSelfIdx += 4;
		}
	}
	if (SelfCards[iSelfIdx + 3] == CARD_TERMINATE)
	{
		if (SelfCards[iSelfIdx + 2] != CARD_TERMINATE)
		{
			if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f))
			{
				cOutCard[0] = SelfCards[iSelfIdx];
				return iSelfIdx;
			}
			else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 2] & 0x0f))
			{
				cOutCard[0] = SelfCards[iSelfIdx + 2];
				return iSelfIdx + 2;
			}
			else
			{
				return		-1;
			}
		}
		else if (SelfCards[iSelfIdx + 1] != CARD_TERMINATE)
		{
			if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f))
			{
				cOutCard[0] = SelfCards[iSelfIdx];
				return iSelfIdx;
			}
			else
			{
				return		-1;
			}
		}
		else if (SelfCards[iSelfIdx] != CARD_TERMINATE)
		{
			if ((SelfCards[iSelfIdx - 1] & 0x0f) != (SelfCards[iSelfIdx] & 0x0f))
			{
				cOutCard[0] = SelfCards[iSelfIdx];
				return iSelfIdx;
			}
			else
			{
				return -1;
			}
		}

		return		-1;
	}
	else
	{
		cOutCard[0] = SelfCards[iSelfIdx];
		return iSelfIdx;
	}


}

int32 OutCardAi::forceGetSingleCard(uint8* SelfCards, uint8* cOutCard, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;

	//炸弹不能拆成单张
	while (SelfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		if ((SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 1] & 0x0f)
			&& (SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 2] & 0x0f)
			&& (SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 3] & 0x0f)
			)
		{
			iSelfIdx += 4;
		}
		else
		{
			break;
		}
	}

	//火箭不能拆成单牌
	if ((SelfCards[iSelfIdx] & 0x0f) == 13 && (SelfCards[iSelfIdx + 1] & 0x0f) == 14)
	{
		return -1;
	}
	if (SelfCards[iSelfIdx] == CARD_TERMINATE)
	{
		return -1;
	}

	cOutCard[0] = SelfCards[iSelfIdx];
	return iSelfIdx;
}

int32 OutCardAi::getPairCard(uint8* SelfCards, uint8* OutCards, int32 CardBegPos)
{
	int32 iSelfIdx = CardBegPos;
	while (SelfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f))
		{
			iSelfIdx++;
		}
		else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 2] & 0x0f))
		{
			break;
		}
		else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 3] & 0x0f))
		{
			iSelfIdx += 3;
		}
		else
		{
			iSelfIdx += 4;
		}
	}
	if (SelfCards[iSelfIdx + 3] == CARD_TERMINATE)
	{
		///不出牌
		if (SelfCards[iSelfIdx + 2] != CARD_TERMINATE)
		{
			if ((SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 1] & 0x0f)
				&& (SelfCards[iSelfIdx + 1] & 0x0f) != (SelfCards[iSelfIdx + 2] & 0x0f))
			{
				OutCards[0] = SelfCards[iSelfIdx];
				OutCards[1] = SelfCards[iSelfIdx + 1];
				return	iSelfIdx;
			}
			else if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f) && (SelfCards[iSelfIdx + 1] & 0x0f) == (SelfCards[iSelfIdx + 2] & 0x0f))
			{
				OutCards[0] = SelfCards[iSelfIdx + 1];
				OutCards[1] = SelfCards[iSelfIdx + 2];
				return	iSelfIdx + 1;
			}
			else
			{
				return -1;
			}
		}
		else if (SelfCards[iSelfIdx + 1] != CARD_TERMINATE)
		{
			if ((SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 1] & 0x0f))
			{
				OutCards[0] = SelfCards[iSelfIdx];
				OutCards[1] = SelfCards[iSelfIdx + 1];
				return	iSelfIdx;
			}
			else
			{
				return -1;
			}
		}
		return -1;
	}
	else
	{
		OutCards[0] = SelfCards[iSelfIdx];
		OutCards[1] = SelfCards[iSelfIdx + 1];
		return	iSelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::forceGetPairCard(uint8* SelfCards, uint8* OutCards, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;
	while (SelfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		//炸弹不能拆出对子
		if ((SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 1] & 0x0f)
			&& (SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 2] & 0x0f)
			&& (SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 3] & 0x0f)
			)
		{
			iSelfIdx += 4;
			continue;
		}

		if ((SelfCards[iSelfIdx] & 0x0f) != (SelfCards[iSelfIdx + 1] & 0x0f))
		{
			iSelfIdx++;
		}
		else
		{
			break;
		}
	}

	uint8 pp[17];

	memcpy(pp, SelfCards, 17);
	if (SelfCards[iSelfIdx] != CARD_TERMINATE && SelfCards[iSelfIdx + 1] != CARD_TERMINATE
		&& (SelfCards[iSelfIdx] & 0x0f) == (SelfCards[iSelfIdx + 1] & 0x0f))
	{
		OutCards[0] = SelfCards[iSelfIdx];
		OutCards[1] = SelfCards[iSelfIdx + 1];
		return iSelfIdx;
	}
	return -1;

}

int32 OutCardAi::getTripleCard(uint8* SelfCards, uint8* OutCards, int32 CardBegPos)
{
	int32 SelfIdx = CardBegPos;
	while (SelfCards[SelfIdx + 3] != CARD_TERMINATE)
	{
		if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 1] & 0x0f))
		{
			SelfIdx++;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 2] & 0x0f))
		{
			SelfIdx += 2;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 3] & 0x0f))
		{
			break;
		}
		else
		{
			SelfIdx += 4;
		}
	}
	if (SelfCards[SelfIdx + 3] == CARD_TERMINATE)
	{
		///不出牌
		if (SelfCards[SelfIdx + 2] != CARD_TERMINATE)
		{
			if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f)
				&& (SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f))
			{
				OutCards[0] = SelfCards[SelfIdx];
				OutCards[1] = SelfCards[SelfIdx + 1];
				OutCards[2] = SelfCards[SelfIdx + 2];
				return SelfIdx;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		OutCards[0] = SelfCards[SelfIdx];
		OutCards[1] = SelfCards[SelfIdx + 1];
		OutCards[2] = SelfCards[SelfIdx + 2];
		return SelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::getBombCard(uint8* SelfCards, uint8* OutCards, int32 CardBegPos)
{
	int32 SelfIdx = CardBegPos;
	while (SelfCards[SelfIdx + 3] != CARD_TERMINATE)
	{
		if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 1] & 0x0f))
		{
			SelfIdx++;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 2] & 0x0f))
		{
			SelfIdx += 2;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) != (SelfCards[SelfIdx + 3] & 0x0f))
		{
			SelfIdx += 3;
		}
		else
		{
			break;
		}
	}
	if (SelfCards[SelfIdx + 3] == CARD_TERMINATE)
	{
		return -1;///不出牌
	}
	else
	{
		OutCards[0] = SelfCards[SelfIdx];
		OutCards[1] = SelfCards[SelfIdx + 1];
		OutCards[2] = SelfCards[SelfIdx + 2];
		OutCards[3] = SelfCards[SelfIdx + 3];
		return	 SelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::getRocketCard(uint8* SelfCards, uint8* OutCards)
{
	int32 iCardIdx = 0;
	while (SelfCards[iCardIdx] != CARD_TERMINATE)
	{
		iCardIdx++;
	}
	if ((SelfCards[iCardIdx - 1] & 0x0f) == 14 && (SelfCards[iCardIdx - 2] & 0x0f) == 13)
	{
		OutCards[0] = SelfCards[iCardIdx - 2];
		OutCards[1] = SelfCards[iCardIdx - 1];
		return iCardIdx - 2;
	}
	else
	{
		return -1;
	}
}

int32 OutCardAi::getSingleProgression(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	while (SelfCards[SelfIdx + 1] != CARD_TERMINATE && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			SelfIdx++;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f))
		{
			SelfIdx++;
		}
		else
		{
			iTmpCount = 0;
			SelfIdx++;
		}
		if (iTmpCount >= CardsNum)
		{
			break;
		}
	}
	if (SelfCards[SelfIdx + 1] == CARD_TERMINATE && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx - 1] & 0x0f) == (SelfCards[SelfIdx] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			SelfIdx++;
		}
	}
	if (iTmpCount < CardsNum)
	{
		return -1;///不出牌
	}
	else
	{
		int32 iIdx = 0;
		for (; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
		///更新手中的牌
	}
}

int32	OutCardAi::getPairProgression(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum < 6 || CardsNum % 2 != 0)
	{
		return -1;
	}
	while (SelfCards[SelfIdx + 2] != CARD_TERMINATE && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f)
			&& (SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 1];
			SelfIdx += 2;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f)
			&& (SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f))
		{
			if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 3] & 0x0f))
			{
				iTmpCount = 0;
				SelfIdx += 4;
			}
			else
			{
				SelfIdx++;
			}
		}
		else
		{
			iTmpCount = 0;
			SelfIdx++;
		}
		if (iTmpCount >= CardsNum)
		{
			break;
		}
	}
	if (SelfCards[SelfIdx + 2] == CARD_TERMINATE && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 1] & 0x0f))
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 1];
			SelfIdx += 2;
		}
	}
	if (iTmpCount < CardsNum)
	{
		return -1;
	}
	else
	{
		int32 iIdx = 0;
		for (; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
		///更新手中的牌
	}
}

int32	OutCardAi::getTripleProgression(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum < 6 || CardsNum % 3 != 0)
	{
		return -1;
	}
	while (SelfCards[SelfIdx + 3] != CARD_TERMINATE && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f)
			&& (SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 3] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 1];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 2];
			SelfIdx += 3;
		}
		else if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f)
			&& (SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 3] & 0x0f))
		{
			iTmpCount = 0;
			SelfIdx += 4;
		}
		else
		{
			iTmpCount = 0;
			SelfIdx++;
		}
		if (iTmpCount >= CardsNum)
		{
			break;
		}
	}
	if ((SelfCards[SelfIdx + 3] == CARD_TERMINATE) && ((SelfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((SelfCards[SelfIdx] & 0x0f) == (SelfCards[SelfIdx + 2] & 0x0f))
		{
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 1];
			pcTmpCards[iTmpCount++] = SelfCards[SelfIdx + 2];
			SelfIdx += 3;
		}
	}
	if (iTmpCount < CardsNum)
	{
		return -1;///不出牌
	}
	else
	{
		int32 iIdx = 0;
		for (; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
		///更新手中的牌
	}
}

int32	OutCardAi::getTripleWithOne(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	int32  iResult = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum != 4)
	{
		return -1;
	}
	if ((SelfIdx = getTripleCard(SelfCards, pcTmpCards, CardBegPos)) >= 0)
	{
		iTmpCount += 3;
		if (getSingleCard(SelfCards, pcTmpCards + iTmpCount, 0) < 0)
		{
			return -1;
		}
		else
		{
			iTmpCount++;
		}
	}
	else
	{
		return -1;
	}

	int32 iIdx = 0;
	for (; iIdx < CardsNum; iIdx++)
	{
		OutCardss[iIdx] = pcTmpCards[iIdx];
	}
	return	 SelfIdx;
	///更新手中的牌
}

int32	OutCardAi::getTripleWithTwo(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	int32  iResult = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum != 5)
	{
		return -1;
	}
	if ((SelfIdx = getTripleCard(SelfCards, pcTmpCards, CardBegPos)) >= 0)
	{
		iTmpCount += 3;
		if (getPairCard(SelfCards, pcTmpCards + iTmpCount, 0) < 0)
		{
			return -1;
		}
		else
		{
			iTmpCount += 2;
		}
	}
	else
	{
		return -1;
	}

	int32 iIdx = 0;
	for (; iIdx < CardsNum; iIdx++)
	{
		OutCardss[iIdx] = pcTmpCards[iIdx];
	}
	return	 SelfIdx;
	///更新手中的牌
}

int32	OutCardAi::getAirPlane(uint8* SelfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	int32  iTripleNum;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum % 4 == 0)
	{
		iTripleNum = CardsNum >> 2;
	}
	else if (CardsNum % 5 == 0)
	{
		iTripleNum = CardsNum / 5;
	}
	else
	{
		return -1;
	}
	if (getTripleProgression(SelfCards, pcTmpCards, CardBegPos, iTripleNum * 3) >= 0)
	{
		int32 iIdx = 0;
		int32 iPos = 0;
		iTmpCount += iTripleNum * 3;
		if (CardsNum % 4 == 0)
		{
			for (iIdx = 0; iIdx < iTripleNum; iIdx++)
			{
				iPos = getSingleCard(SelfCards, pcTmpCards + iTmpCount, iPos);
				if (iPos < 0)
				{
					break;
				}
				iTmpCount++;
				iPos++;
			}
			if (iIdx < iTripleNum)
			{
				return -1;
			}
		}
		else
		{
			for (iIdx = 0; iIdx < iTripleNum; iIdx++)
			{
				iPos = getPairCard(SelfCards, pcTmpCards + iTmpCount, iPos);
				if (iPos < 0)
				{
					break;
				}
				iTmpCount += 2;
				iPos += 2;
			}
			if (iIdx < iTripleNum)
			{
				return -1;
			}
		}
		for (iIdx = 0; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
	}
	else
	{
		return -1;
	}

}

