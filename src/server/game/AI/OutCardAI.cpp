#include "OutCardAI.h"

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
	player->_cardType = player->_curOutCardType;
	memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));

	if (player->_curOutCardsPlayer == player)
	{
		player->_cardType = getPlayOutCardFirst(player);
	}
	else
	{
		getPlayOutCard(player, &player->_cardType);
	}
}

void OutCardAi::updateCardsFace(uint8* selfCards, uint8* outCards)
{
	int32 iCardIdx = 0;
	if (*outCards == CARD_TERMINATE)
	{
		return ;
	}
	while (outCards[iCardIdx] != CARD_TERMINATE)
	{
		uint8 cTmpCard = outCards[iCardIdx];
		rearrangeCards(selfCards, cTmpCard);
		iCardIdx++;
	}
}

void OutCardAi::rearrangeCards(uint8* selfCards, uint8 cCard)
{
	int32 iCardIdx = 0;
	for (; selfCards[iCardIdx] != CARD_TERMINATE; iCardIdx++)
	{
		if (selfCards[iCardIdx] == cCard)
		{
			break;
		}
	}
	for (; selfCards[iCardIdx] != CARD_TERMINATE; iCardIdx++)
	{
		selfCards[iCardIdx] = selfCards[iCardIdx + 1];
	}
}

void OutCardAi::arraggeCard(uint8 cards[], uint32 num)
{
	int32 iIdx = 0;
	uint8 tmpCards[24];
	memcpy(tmpCards, cards, sizeof(tmpCards));
	memset(cards, CARD_TERMINATE, num);
	for (; iIdx < num; iIdx++)
	{
		insertCard(cards, tmpCards[iIdx], iIdx);
	}
}

void OutCardAi::insertCard(uint8 cards[], uint8 card, uint32 cardSortNum)
{
	int iIdx = cardSortNum - 1;

	for (; iIdx >= 0; iIdx--)
	{
		if ((cards[iIdx] & 0x0f) > (card & 0x0f))
		{
			cards[iIdx + 1] = cards[iIdx];
		}
		else
		{
			break;
		}
	}
	cards[iIdx + 1] = card;
}

void OutCardAi::SaveOutCards(uint8 * allOutCards, uint8 * curOutCards)
{
	while (*allOutCards != CARD_TERMINATE)
	{
		++allOutCards;
	}
	while (*curOutCards != CARD_TERMINATE)
	{
		*allOutCards++ = *curOutCards++;
	}
}

/*
 * out card logic
 */

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
	Player *their = (player->getPlayerGameType() == player->_left->getPlayerGameType() ? player->_left : player->_right);

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

CardType OutCardAi::getPlayOutCardFirst(uint8 selfCards[], uint8 outCards[])
{
	CardType cardType = CARD_TYPE_PASS;

	int32 CardsNum = 0;
	while (selfCards[CardsNum] != CARD_TERMINATE)
	{
		CardsNum++;
	}

	//if(CardsNum <= 10)
	{
		if (CardsNum == 10)
		{
			if (getAirPlane(selfCards, outCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(selfCards, outCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(selfCards, outCards, 0, CardsNum) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 9)
		{
			if (getTripleProgression(selfCards, outCards, 0, 9) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getSingleProgression(selfCards, outCards, 0, 9) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 8)
		{
			if (getAirPlane(selfCards, outCards, 0, 8) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(selfCards, outCards, 0, 8) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(selfCards, outCards, 0, 8) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 6)
		{
			if (getTripleProgression(selfCards, outCards, 0, 6) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getPairProgression(selfCards, outCards, 0, 6) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
			if (getSingleProgression(selfCards, outCards, 0, 6) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
		}
		if (CardsNum >= 5)
		{
			if (getSingleProgression(selfCards, outCards, 0, 5) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}

			if (getTripleWithTwo(selfCards, outCards, 0, 5) >= 0)
			{
				return CARD_TYPE_TRIPLE_TWO;
			}

		}
		if (CardsNum >= 4)
		{
			if (getTripleWithOne(selfCards, outCards, 0, 4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
		}
	}
	if ((selfCards[0] & 0x0f) == 13 && (selfCards[1] & 0x0f) == 14 && selfCards[2] == CARD_TERMINATE)
	{
		outCards[0] = selfCards[0];
		outCards[1] = selfCards[1];
		return CARD_TYPE_ROCKET;
	}

	int32 iIdx = 0;

	while (selfCards[iIdx] != CARD_TERMINATE)
	{
		if (selfCards[iIdx + 1] == CARD_TERMINATE)
		{
			outCards[0] = selfCards[iIdx];
			cardType = CARD_TYPE_SINGLE;
			break;
		}
		else if ((selfCards[iIdx] & 0x0f) != (selfCards[iIdx + 1] & 0x0f))
		{
			outCards[0] = selfCards[iIdx];
			cardType = CARD_TYPE_SINGLE;
			break;
		}
		else if (selfCards[iIdx + 2] == CARD_TERMINATE)
		{
			outCards[0] = selfCards[iIdx];
			outCards[1] = selfCards[iIdx + 1];
			cardType = CARD_TYPE_PAIR;
			break;
		}
		else if ((selfCards[iIdx] & 0x0f) != (selfCards[iIdx + 2] & 0x0f))
		{
			outCards[0] = selfCards[iIdx];
			outCards[1] = selfCards[iIdx + 1];
			cardType = CARD_TYPE_PAIR;
			break;
		}
		else if (selfCards[iIdx + 3] == CARD_TERMINATE)
		{
			outCards[0] = selfCards[iIdx];
			outCards[1] = selfCards[iIdx + 1];
			outCards[2] = selfCards[iIdx + 2];
			cardType = CARD_TYPE_TRPILE;
			break;
		}
		else if ((selfCards[iIdx] & 0x0f) != (selfCards[iIdx + 3] & 0x0f))
		{
			outCards[0] = selfCards[iIdx];
			outCards[1] = selfCards[iIdx + 1];
			outCards[2] = selfCards[iIdx + 2];
			int32 iResult = getSingleCard(selfCards, outCards + 3, 0);
			if (iResult >= 0)
			{
				cardType = CARD_TYPE_TRIPLE_ONE;
				break;
			}
			else
			{
				iResult = getPairCard(selfCards, outCards + 3, 0);
				if (iResult >= 0)
				{
					cardType = CARD_TYPE_TRIPLE_TWO;

					break;
				}
				else
				{
					cardType = CARD_TYPE_TRPILE;
					break;
				}
			}
		}
		else
		{
			iIdx += 4;
		}
	}
	if (selfCards[iIdx] != CARD_TERMINATE)
	{
		return cardType;
	}
	else
	{
		outCards[0] = selfCards[0];
		outCards[1] = selfCards[1];
		outCards[2] = selfCards[2];
		outCards[3] = selfCards[3];
		return CARD_TYPE_BOMB;
	}
}

int32 OutCardAi::getPlayOutCard(Player * player,CardType *cardType)
{
	Player *nextPlayer = player->_right;
	/// 下家是对家
	if (player->getPlayerGameType() != nextPlayer->getPlayerGameType())
	{
		/// 下家手中剩余牌数
		int32 iUserNextCardsNum = getCardsNumber(nextPlayer->_cards);

		/// 当前打出的牌面的牌数
		int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
		/// 当前出牌用户，剩余的牌数
		int32 iCardsCurNum = getCardsNumber(player->_curOutCardsPlayer->_cards);

		/// 当前我的牌面的牌数
		int32 iCardsSelfNum = getCardsNumber(player->_cards);

		/// 如果当前牌面数和地主手里的剩余牌数相等(则要出大牌，别放走对家)
		if (iUserNextCardsNum == iCardsRecvNum)
		{
			/// 当前牌面是单支
			if (1 == iCardsRecvNum || 2 == iCardsRecvNum || 3 == iCardsRecvNum)
			{

				/// 当前已经打出牌的用户是自家,且其手中的牌小于3张
				if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType()
					&& iCardsCurNum < 3)
				{
					memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
				}
				else if (CARD_TYPE_PASS != playOutMaxCardsByType(player->_cards, player->_outCards, player->_curOutCardType, iCardsSelfNum))
				{
					/// 当前收到的牌的主牌信息
					int32 iMainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum) & 0x0f);

					/// 当前自己打出的牌的主牌信息
					int32 iMainCardNumSelf = (getMainCardFromRecv(player->_curOutCardType, player->_outCards, iCardsRecvNum) & 0x0f);
					if (iMainCardNumSelf > iMainCardNumRecv && iMainCardNumRecv != 61)
					{
						return 1;
					}
				}
			}
		}
	}

	uint8 cardsCurTmp[80];
	memset(cardsCurTmp, 0, sizeof(cardsCurTmp));

	cardsCurTmp[79] = 0;

	/// 获得本局我要打出的牌信息
	memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));

	bool bSplitCard = bSplitCards(player);//是否要拆牌

	int32 iMainCardsIdx = getPlayOutCard(player->_curOutCardType, player->_curOutCards
		, player->_cards, player->_outCards, bSplitCard);

	if (CARD_TERMINATE == player->_outCards[0])
	{
		memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
		*cardType = CARD_TYPE_PASS;
		return 1;
	}

	getPlayOutCardType(player->_outCards, cardType);
	/// 如果我是一手牌()
	/// 获得自家手中剩余牌数
	int32 iCardsSelfNum = getCardsNumber(player->_cards);

	/// 如果手中牌全部可以打住(一手出完，赢了)
	if (iCardsSelfNum == getCardsNumber(player->_outCards))
	{
		/// 正常出牌，赢了
	}
	else
	{
		/// 当前已经打出牌的用户是自家
		if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType())
		{
			//当前出牌用户手中的牌数
			int32 iCardCurNumber = getCardsNumber(player->_curOutCardsPlayer->_cards);
			/// 当前打出的牌面的牌数
			int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);

			//如果自家打出的牌和他手中的牌数一样或者手中只剩1,2张牌，我不出牌
			if (iCardCurNumber == iCardsRecvNum || iCardCurNumber == 1 || iCardCurNumber == 2)
			{
				memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
				*cardType = CARD_TYPE_PASS;
			}
			/// 如果我的主牌是炸弹
			if (CARD_TYPE_ROCKET == *cardType || CARD_TYPE_BOMB == *cardType)
			{
				// 			/// 获得自家手中剩余牌数
				// 			int32 iCardsSelfNum = getCardsNum(pkQueueDeskDataNode->pcCardsTotal[iQueueUserIdx]);
				if (4 == iCardsSelfNum || 2 == iCardsSelfNum)//???主牌是火箭，手里有四张牌
				{
					/// 正常出牌，赢了
				}
				else
				{
					/// 如果是火箭(双王)
					if (CARD_TYPE_ROCKET == *cardType)
					{
						if (3 == iCardsSelfNum)
						{
							/// 正常出牌，赢了
						}
						else
						{
							/// 我不出牌 pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
					}
					else /// 我不出牌 pass
					{
						memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
						*cardType = CARD_TYPE_PASS;
					}
				}
			}
			else /// 如果我的主牌不是炸弹
			{
				/// 获得我的主牌信息
				int32 iMainCardsNumSelf = (player->_cards[iMainCardsIdx] & 0x0f);

				/// 获得当前牌面的主牌信息
				int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
				int32 iMainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum) & 0x0f);

				// 			/// 获得自己手中剩余牌数
				// 			int32 iCardsSelfNum = getCardsNum(pkQueueDeskDataNode->pcCardsTotal[iQueueUserIdx]);

				/// 如果自己手中牌数量和打出的牌数量一样
				if (iCardsRecvNum == iCardsSelfNum)
				{
					/// 正常出牌，赢了
				}
				else
				{
					if (CARD_TYPE_SINGLE == player->_curOutCardType)/// 单支
					{
						///  当前牌面主牌 >= A
						if (11 <= iMainCardNumRecv)
						{
							/// 我不出牌 pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else ///  当前牌面主牌 < A
						{
							/// 我的主牌 <= A
							if (11 >= iMainCardsNumSelf)
							{
								/// 正常出牌
							}
							else /// 我的主牌 > A
							{
								/// 我不出牌 pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else if (CARD_TYPE_PAIR == player->_curOutCardType) /// 对子
					{
						/// 当前牌面主牌 >= K
						if (10 <= iMainCardNumRecv)
						{
							/// 我不出牌 pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else /// 当前牌面主牌 < K
						{
							/// 我的主牌 < K
							if (10 > iMainCardsNumSelf)
							{
								/// 正常出牌
							}
							else /// 我的主牌 >= K
							{
								/// 我不出牌 pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else if (CARD_TYPE_TRPILE == player->_curOutCardType
						|| CARD_TYPE_TRIPLE_ONE == player->_curOutCardType
						|| CARD_TYPE_TRIPLE_TWO == player->_curOutCardType) /// 三支
					{
						/// 当前牌面主牌 >= K
						if (10 <= iMainCardNumRecv)
						{
							/// 我不出牌 pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else /// 当前牌面主牌 < K
						{
							/// 我的主牌 < J
							if (8 > iMainCardsNumSelf)
							{
								/// 正常出牌
							}
							else /// 我的主牌 >= J
							{
								/// 我不出牌 pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else /// 当前牌面其它牌型
					{
						/// 我不出牌 pass
						memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
						*cardType = CARD_TYPE_PASS;
					}
				}
			}
		}
		else /// 当前已经打出牌的用户是对家
		{
			///DEBUG(4, "User PlayOutCard  303......\n");
			/// 获得当前牌面的主牌信息
			int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
			int32 iMainCardNumRecv = getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum);

			/// 炸弹
			if (CARD_TYPE_ROCKET == player->_curOutCardType
				|| CARD_TYPE_BOMB == player->_curOutCardType)
			{
				/// 我正常出牌(我有炸弹就要出)
			}
			else /// 当前牌面其它牌型
			{
				/// 如果我的主牌是炸弹
				if (CARD_TYPE_ROCKET == *cardType || CARD_TYPE_BOMB == *cardType)
				{
					/// 如果对家手中就剩了1，2张牌，我就出牌(防止对家剩一手牌走掉)
					int32 iUserCardsNumEnemy = getCardsNumber(player->_curOutCardsPlayer->_cards);
					if (1 == iUserCardsNumEnemy || 2 == iUserCardsNumEnemy)
					{
						/// 我正常出牌(炸弹就要出)
					}
					else
					{
						/// 如果我下家是自家
						if (player->getPlayerGameType() == player->_right->getPlayerGameType())
						{
							/// 获得下家手中牌的个数
							int32 iCardsSameFamilyNum = getCardsNumber(player->_right->_cards);

							/// 如果我下家的手中牌数少于当前打出的牌数,(我能打就要打)
							if (iCardsSameFamilyNum < iCardsRecvNum)
							{
								/// 我正常出牌(炸弹就要出)
							}
							else /// 我不出牌 pass
							{
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
						else /// 如果我下家是对家
						{
							if (iUserCardsNumEnemy > 10)///如果对家的牌大于10张，不出牌
							{
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
							/// 我正常出牌(炸弹就要出)
						}
					}
				}
				else /// 如果我的主牌不是炸弹
				{
					////DEBUG(4, "User PlayOutCard  306......\n");
					/// 我正常出牌
				}
			}
		}
	}

	return 1;
}

int OutCardAi::getPlayOutCard(CardType cardType, uint8 recvCard[], uint8 selfCards[], uint8 outCards[], bool bSplitCard)
{
	int iSelfIdx = 0;
	int iRecvIdx = 0;
	int iCardsRecvNum = 0;
	int iMainCard;
	int	iResult = -1;///>=0 表示得到的牌的起始索引

	int iCardsNum = 0;
	while (selfCards[iCardsNum] != CARD_TERMINATE)
	{
		iCardsNum++;
	}
	if (cardType == CARD_TYPE_ROCKET)
	{
		return iResult;
	}
	while (recvCard[iCardsRecvNum] != CARD_TERMINATE)
	{
		iCardsRecvNum++;
	}
	iMainCard = getMainCardFromRecv(cardType, recvCard, iCardsRecvNum);
	for (; selfCards[iSelfIdx] != CARD_TERMINATE; iSelfIdx++)
	{
		if ((selfCards[iSelfIdx] & 0x0f) > (iMainCard & 0x0f))
		{
			break;
		}
	}

	if ((selfCards[iSelfIdx] == CARD_TERMINATE))
	{
		if (cardType != CARD_TYPE_BOMB)
		{
			iSelfIdx = 0;
			iResult = getBombCard(selfCards, outCards, iSelfIdx);
			return 	iResult;
		}
		else
		{
			return iResult;
		}
	}

	switch (cardType)
	{
		case CARD_TYPE_SINGLE:
		{
			iResult = getSingleCard(selfCards, outCards, iSelfIdx);

			if (iResult == -1 && bSplitCard)//如果没有找出单张，且可以拆牌，拆出一个单张
				iResult = forceGetSingleCard(selfCards, outCards, iSelfIdx);

		}
			break;
		case CARD_TYPE_PAIR:
		{
			iResult = getPairCard(selfCards, outCards, iSelfIdx);

			 if (iResult == -1 && bSplitCard)//如果没有找出对子，且可以拆牌，拆出一个对子
			  iResult = forceGetPairCard(selfCards, outCards, iSelfIdx);
		}
		break;
		case CARD_TYPE_TRPILE:
		{
			if (iCardsNum > 2)
			 iResult = getTripleCard(selfCards, outCards, iSelfIdx);
		}
			break;
		case CARD_TYPE_TRIPLE_ONE:
		{
			if (iCardsNum > 3)
			   iResult = getTripleWithOne(selfCards, outCards, iSelfIdx, iCardsRecvNum);

		}
		break;
		case CARD_TYPE_TRIPLE_TWO:
		{
			if (iCardsNum > 4)
			   iResult = getTripleWithTwo(selfCards, outCards, iSelfIdx, iCardsRecvNum);
		}
			break;
		case CARD_TYPE_SINGLE_PROGRESSION:
		{
			if (iCardsNum > 4)
			  iResult = getSingleProgression(selfCards, outCards, iSelfIdx, iCardsRecvNum);
		}
		break;
		case CARD_TYPE_PAIR_PROGRESSION:
		{
		  if (iCardsNum > 5)
			iResult = getPairProgression(selfCards, outCards, iSelfIdx, iCardsRecvNum);
		}
		break;
		case CARD_TYPE_TRIPLE_PROGRESSION:
		{
		 if (iCardsNum > 5)
			  iResult = getTripleProgression(selfCards, outCards, iSelfIdx, iCardsRecvNum);
		}
		break;
		case CARD_TYPE_AIRPLANE:
		{
		 if (iCardsNum > 7)
		   iResult = getAirPlane(selfCards, outCards, iSelfIdx, iCardsRecvNum);
		}
		break;
		case CARD_TYPE_FOUR_TWO:
		{
		  iResult = getBombCard(selfCards, outCards, 0);
		}
		case CARD_TYPE_BOMB:
		{
		  iResult = getBombCard(selfCards, outCards, iSelfIdx);
		}
		break;
		default:
			break;
	}

	if (iResult < 0 && cardType != CARD_TYPE_BOMB)
	{
		iSelfIdx = 0;
		iResult = getBombCard(selfCards, outCards, iSelfIdx);
	}
	if (iResult < 0)
	{
		return getRocketCard(selfCards, outCards);
	}
	else
	{
		return iResult;
	}
}

CardType OutCardAi::IsSeriesCard(uint8 selfCards[], uint8 outCards[])
{
	uint32 SelfCardsNum = 0;
	while (selfCards[SelfCardsNum] != CARD_TERMINATE)
	{
		SelfCardsNum++;
	}

	if (SelfCardsNum <= 10)
	{
		if (SelfCardsNum == 10 || SelfCardsNum == 8)
		{
			if (getAirPlane(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_AIRPLANE;
			}
			if (getPairProgression(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
		}
		if (SelfCardsNum == 9)
		{
			if (getTripleProgression(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
		}
		if (SelfCardsNum == 6)
		{
			if (getTripleProgression(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_PROGRESSION;
			}
			if (getPairProgression(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_PAIR_PROGRESSION;
			}
		}
		if (SelfCardsNum >= 5)
		{
			if (getSingleProgression(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_SINGLE_PROGRESSION;
			}
			if (SelfCardsNum == 5)
			{
				if (getTripleWithTwo(selfCards, outCards, 0, SelfCardsNum) >= 0)
				{
					return CARD_TYPE_TRIPLE_TWO;
				}
			}
		}
		if (SelfCardsNum == 4)
		{
			if (getTripleWithOne(selfCards, outCards, 0, SelfCardsNum) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
		}
	}
	return CARD_TYPE_PASS;
}

CardType OutCardAi::playOutCardsNotByType(uint8 selfCards[], uint8 outCards[], int32 CardType, int32 SelfCardsNum)
{
	switch(CardType)
	{
	case CARD_TYPE_SINGLE:
		{
			if(getTripleWithOne(selfCards,outCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getTripleCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else if(getPairCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				outCards[0]= selfCards[SelfCardsNum -1];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_PAIR:
		{
			if(getTripleWithOne(selfCards,outCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getTripleCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else
			{
				outCards[0]= selfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_TRPILE:
		{
			if(getTripleWithOne(selfCards,outCards,0,4) >= 0)
			{
				return CARD_TYPE_TRIPLE_ONE;
			}
			else if(getPairCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				outCards[0]= selfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	case CARD_TYPE_TRIPLE_ONE:
		{
			if(getTripleCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_TRPILE;
			}
			else if(getPairCard(selfCards,outCards,0) >= 0)
			{
				return CARD_TYPE_PAIR;
			}
			else
			{
				outCards[0]= selfCards[0];
				return CARD_TYPE_SINGLE;
			}
		}
	}

	return CARD_TYPE_PASS;
}

CardType OutCardAi::playOutMaxCardsByType(uint8 selfCards[], uint8 outCards[], CardType cardType, int32 selfCardsNum)
{
	switch (cardType)
	{
		case CARD_TYPE_SINGLE:
		{
			 outCards[0] = selfCards[selfCardsNum - 1];
			return CARD_TYPE_SINGLE;
		}
		case CARD_TYPE_PAIR:
		{
			 uint8 tmpCards[4];
			 memset(tmpCards, CARD_TERMINATE, 4);
			 int32 iCardIdx = getPairCard(selfCards, tmpCards, 0);
			 while (iCardIdx >= 0)
			 {
					outCards[0] = tmpCards[0];
				    outCards[1] = tmpCards[1];
					iCardIdx = getPairCard(selfCards, tmpCards, iCardIdx + 2);
			 }
			if (outCards[0] == CARD_TERMINATE)
				return CARD_TYPE_PASS;
            else
				return CARD_TYPE_PAIR;
		}
	    case CARD_TYPE_TRPILE:
	    {
			uint8 tmpCards[4];
			memset(tmpCards, 100, 4);
			int32 cardIdx = getTripleCard(selfCards, tmpCards, 0);
			while (cardIdx >= 0)
			{
			 outCards[0] = tmpCards[0];
			 outCards[1] = tmpCards[1];
			 outCards[2] = tmpCards[2];
			 cardIdx = getPairCard(selfCards, tmpCards, cardIdx + 3);
			}
			 if (outCards[0] == CARD_TERMINATE)
				 return CARD_TYPE_PASS;
            else
                 return CARD_TYPE_TRPILE;
	    }
	}
	return CARD_TYPE_PASS;
}

int32 OutCardAi::getMainCardFromRecv(CardType cardType, uint8* cardsRecv, int32 cardsRecvNum)
{
	switch (cardType)
	{
		case CARD_TYPE_PASS:
		{
		  return -1;
		}
		case CARD_TYPE_SINGLE:
		case CARD_TYPE_PAIR:
		case CARD_TYPE_TRPILE:
		case CARD_TYPE_BOMB:
		case CARD_TYPE_SINGLE_PROGRESSION:
		case CARD_TYPE_PAIR_PROGRESSION:
		case CARD_TYPE_TRIPLE_PROGRESSION:
		{
		 return cardsRecv[0];
		}
		case CARD_TYPE_TRIPLE_ONE:
		{
			return judgeTripleWithOne(cardsRecv, cardsRecvNum);
		}
		case CARD_TYPE_TRIPLE_TWO:
		{
			return judgeTripleWithTwo(cardsRecv, cardsRecvNum);
		}
		case CARD_TYPE_AIRPLANE:
		{
			return judgeAirPlane(cardsRecv, cardsRecvNum);
		}
		case CARD_TYPE_FOUR_TWO:
		{
			return judgeFourWithTwo(cardsRecv, cardsRecvNum);
		}
	}
	return -1;
}

bool OutCardAi::bSplitCards(Player * player)
{
	//出牌用户是自家，不拆牌
	if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType())
	{
		return false;
	}

	/// 当前打出的牌面的牌数
	int cardsRecvNum = getCardsNumber(player->_curOutCards);
	int mainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, cardsRecvNum) & 0x0f);
	Player * playerNext = player->_right;

	//如果对方出牌的主牌小于10，且下家是自家，也不拆牌
	if (mainCardNumRecv < 7
		&& player->getPlayerGameType() == playerNext->getPlayerGameType())
	{
		return false;
	}
	return true;
}

int32 OutCardAi::getPlayOutCardType(uint8 outCards[], CardType* cardType)
{
	int	iCardCount = 0;
	while (CARD_TERMINATE != outCards[iCardCount])
	{
		iCardCount++;
	}

	if (iCardCount == 2)
	{
		if (((outCards[0] & 0x0f) == 13 && (outCards[1] & 0x0f) == 14)
			|| ((outCards[1] & 0x0f) == 13 && (outCards[0] & 0x0f) == 14))
		{
			*cardType = CARD_TYPE_ROCKET;
		}
	}
	else if (iCardCount == 4)
	{
		if ((outCards[0] & 0x0f) == (outCards[1] & 0x0f)
			&& (outCards[0] & 0x0f) == (outCards[2] & 0x0f)
			&& (outCards[0] & 0x0f) == (outCards[3] & 0x0f))
		{
			*cardType = CARD_TYPE_BOMB;
		}
	}

	return 1;
}

/*
 * getCard
 */
int32 OutCardAi::getSingleCard(uint8* selfCards, uint8* cOutCard, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;
	while (selfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{//0,1,2,3
		if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f))
		{
			break;
		}
		else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 2] & 0x0f))
		{
			iSelfIdx += 2;
		}
		else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 3] & 0x0f))
		{
			iSelfIdx += 3;
		}
		else
		{
			iSelfIdx += 4;
		}
	}
	if (selfCards[iSelfIdx + 3] == CARD_TERMINATE)
	{
		if (selfCards[iSelfIdx + 2] != CARD_TERMINATE)
		{
			if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f))
			{
				cOutCard[0] = selfCards[iSelfIdx];
				return iSelfIdx;
			}
			else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 2] & 0x0f))
			{
				cOutCard[0] = selfCards[iSelfIdx + 2];
				return iSelfIdx + 2;
			}
			else
			{
				return		-1;
			}
		}
		else if (selfCards[iSelfIdx + 1] != CARD_TERMINATE)
		{
			if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f))
			{
				cOutCard[0] = selfCards[iSelfIdx];
				return iSelfIdx;
			}
			else
			{
				return		-1;
			}
		}
		else if (selfCards[iSelfIdx] != CARD_TERMINATE)
		{
			if ((selfCards[iSelfIdx - 1] & 0x0f) != (selfCards[iSelfIdx] & 0x0f))
			{
				cOutCard[0] = selfCards[iSelfIdx];
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
		cOutCard[0] = selfCards[iSelfIdx];
		return iSelfIdx;
	}
}

int32 OutCardAi::forceGetSingleCard(uint8* selfCards, uint8* cOutCard, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;

	//炸弹不能拆成单张
	while (selfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		if ((selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 1] & 0x0f)
			&& (selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 2] & 0x0f)
			&& (selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 3] & 0x0f)
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
	if ((selfCards[iSelfIdx] & 0x0f) == 13 && (selfCards[iSelfIdx + 1] & 0x0f) == 14)
	{
		return -1;
	}
	if (selfCards[iSelfIdx] == CARD_TERMINATE)
	{
		return -1;
	}

	cOutCard[0] = selfCards[iSelfIdx];
	return iSelfIdx;
}

int32 OutCardAi::getPairCard(uint8* selfCards, uint8* outCards, int32 CardBegPos)
{
	int32 iSelfIdx = CardBegPos;
	while (selfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f))
		{
			iSelfIdx++;
		}
		else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 2] & 0x0f))
		{
			break;
		}
		else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 3] & 0x0f))
		{
			iSelfIdx += 3;
		}
		else
		{
			iSelfIdx += 4;
		}
	}
	if (selfCards[iSelfIdx + 3] == CARD_TERMINATE)
	{
		///不出牌
		if (selfCards[iSelfIdx + 2] != CARD_TERMINATE)
		{
			if ((selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 1] & 0x0f)
				&& (selfCards[iSelfIdx + 1] & 0x0f) != (selfCards[iSelfIdx + 2] & 0x0f))
			{
				outCards[0] = selfCards[iSelfIdx];
				outCards[1] = selfCards[iSelfIdx + 1];
				return	iSelfIdx;
			}
			else if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f) && (selfCards[iSelfIdx + 1] & 0x0f) == (selfCards[iSelfIdx + 2] & 0x0f))
			{
				outCards[0] = selfCards[iSelfIdx + 1];
				outCards[1] = selfCards[iSelfIdx + 2];
				return	iSelfIdx + 1;
			}
			else
			{
				return -1;
			}
		}
		else if (selfCards[iSelfIdx + 1] != CARD_TERMINATE)
		{
			if ((selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 1] & 0x0f))
			{
				outCards[0] = selfCards[iSelfIdx];
				outCards[1] = selfCards[iSelfIdx + 1];
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
		outCards[0] = selfCards[iSelfIdx];
		outCards[1] = selfCards[iSelfIdx + 1];
		return	iSelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::forceGetPairCard(uint8* selfCards, uint8* outCards, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;
	while (selfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		//炸弹不能拆出对子
		if ((selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 1] & 0x0f)
			&& (selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 2] & 0x0f)
			&& (selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 3] & 0x0f)
			)
		{
			iSelfIdx += 4;
			continue;
		}

		if ((selfCards[iSelfIdx] & 0x0f) != (selfCards[iSelfIdx + 1] & 0x0f))
		{
			iSelfIdx++;
		}
		else
		{
			break;
		}
	}

	uint8 pp[17];

	memcpy(pp, selfCards, 17);
	if (selfCards[iSelfIdx] != CARD_TERMINATE && selfCards[iSelfIdx + 1] != CARD_TERMINATE
		&& (selfCards[iSelfIdx] & 0x0f) == (selfCards[iSelfIdx + 1] & 0x0f))
	{
		outCards[0] = selfCards[iSelfIdx];
		outCards[1] = selfCards[iSelfIdx + 1];
		return iSelfIdx;
	}
	return -1;

}

int32 OutCardAi::getTripleCard(uint8* selfCards, uint8* outCards, int32 CardBegPos)
{
	int32 SelfIdx = CardBegPos;
	while (selfCards[SelfIdx + 3] != CARD_TERMINATE)
	{
		if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 1] & 0x0f))
		{
			SelfIdx++;
		}
		else if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 2] & 0x0f))
		{
			SelfIdx += 2;
		}
		else if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 3] & 0x0f))
		{
			break;
		}
		else
		{
			SelfIdx += 4;
		}
	}
	if (selfCards[SelfIdx + 3] == CARD_TERMINATE)
	{
		///不出牌
		if (selfCards[SelfIdx + 2] != CARD_TERMINATE)
		{
			if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f)
				&& (selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f))
			{
				outCards[0] = selfCards[SelfIdx];
				outCards[1] = selfCards[SelfIdx + 1];
				outCards[2] = selfCards[SelfIdx + 2];
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
		outCards[0] = selfCards[SelfIdx];
		outCards[1] = selfCards[SelfIdx + 1];
		outCards[2] = selfCards[SelfIdx + 2];
		return SelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::getBombCard(uint8* selfCards, uint8* outCards, int32 CardBegPos)
{
	int32 SelfIdx = CardBegPos;
	while (selfCards[SelfIdx + 3] != CARD_TERMINATE)
	{
		if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 1] & 0x0f))
		{
			SelfIdx++;
		}
		else if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 2] & 0x0f))
		{
			SelfIdx += 2;
		}
		else if ((selfCards[SelfIdx] & 0x0f) != (selfCards[SelfIdx + 3] & 0x0f))
		{
			SelfIdx += 3;
		}
		else
		{
			break;
		}
	}
	if (selfCards[SelfIdx + 3] == CARD_TERMINATE)
	{
		return -1;///不出牌
	}
	else
	{
		outCards[0] = selfCards[SelfIdx];
		outCards[1] = selfCards[SelfIdx + 1];
		outCards[2] = selfCards[SelfIdx + 2];
		outCards[3] = selfCards[SelfIdx + 3];
		return	 SelfIdx;
		///更新手中的牌
	}
}

int32 OutCardAi::getRocketCard(uint8* selfCards, uint8* outCards)
{
	int32 iCardIdx = 0;
	while (selfCards[iCardIdx] != CARD_TERMINATE)
	{
		iCardIdx++;
	}
	if ((selfCards[iCardIdx - 1] & 0x0f) == 14 && (selfCards[iCardIdx - 2] & 0x0f) == 13)
	{
		outCards[0] = selfCards[iCardIdx - 2];
		outCards[1] = selfCards[iCardIdx - 1];
		return iCardIdx - 2;
	}
	else
	{
		return -1;
	}
}

int32 OutCardAi::getSingleProgression(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	while (selfCards[SelfIdx + 1] != CARD_TERMINATE && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
			SelfIdx++;
		}
		else if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f))
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
	if (selfCards[SelfIdx + 1] == CARD_TERMINATE && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx - 1] & 0x0f) == (selfCards[SelfIdx] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
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

int32 OutCardAi::getPairProgression(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum < 6 || CardsNum % 2 != 0)
	{
		return -1;
	}
	while (selfCards[SelfIdx + 2] != CARD_TERMINATE && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f)
			&& (selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 1];
			SelfIdx += 2;
		}
		else if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f)
			&& (selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f))
		{
			if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 3] & 0x0f))
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
	if (selfCards[SelfIdx + 2] == CARD_TERMINATE && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 1] & 0x0f))
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 1];
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

int32 OutCardAi::getTripleProgression(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
{
	int32 SelfIdx = CardBegPos;
	uint8 pcTmpCards[20];
	int32	 iTmpCount = 0;
	memset(pcTmpCards, CARD_TERMINATE, 20);
	if (CardsNum < 6 || CardsNum % 3 != 0)
	{
		return -1;
	}
	while (selfCards[SelfIdx + 3] != CARD_TERMINATE && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f)
			&& (selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 3] & 0x0f) - 1)
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 1];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 2];
			SelfIdx += 3;
		}
		else if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f)
			&& (selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 3] & 0x0f))
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
	if ((selfCards[SelfIdx + 3] == CARD_TERMINATE) && ((selfCards[SelfIdx] & 0x0f) < 12))
	{
		if ((selfCards[SelfIdx] & 0x0f) == (selfCards[SelfIdx + 2] & 0x0f))
		{
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 1];
			pcTmpCards[iTmpCount++] = selfCards[SelfIdx + 2];
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

int32 OutCardAi::getTripleWithOne(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
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
	if ((SelfIdx = getTripleCard(selfCards, pcTmpCards, CardBegPos)) >= 0)
	{
		iTmpCount += 3;
		if (getSingleCard(selfCards, pcTmpCards + iTmpCount, 0) < 0)
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

int32 OutCardAi::getTripleWithTwo(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
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
	if ((SelfIdx = getTripleCard(selfCards, pcTmpCards, CardBegPos)) >= 0)
	{
		iTmpCount += 3;
		if (getPairCard(selfCards, pcTmpCards + iTmpCount, 0) < 0)
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

int32 OutCardAi::getAirPlane(uint8* selfCards, uint8* OutCardss, int32 CardBegPos, int32 CardsNum)
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
	if (getTripleProgression(selfCards, pcTmpCards, CardBegPos, iTripleNum * 3) >= 0)
	{
		int32 iIdx = 0;
		int32 iPos = 0;
		iTmpCount += iTripleNum * 3;
		if (CardsNum % 4 == 0)
		{
			for (iIdx = 0; iIdx < iTripleNum; iIdx++)
			{
				iPos = getSingleCard(selfCards, pcTmpCards + iTmpCount, iPos);
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
				iPos = getPairCard(selfCards, pcTmpCards + iTmpCount, iPos);
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


/*
* judgeCard
*/

int32 OutCardAi::judgeSingleProgression(uint8* cards, int32 cardsNum)
{
	if (cardsNum < 5)
	{
		return -1;
	}
	int32 iIdxCount = 1;
	for (; iIdxCount < cardsNum; iIdxCount++)
	{
		if ((cards[0] & 0x0f) + iIdxCount != (cards[iIdxCount] & 0x0f)
			|| (cards[iIdxCount] & 0x0f) >= 12)
		{
			break;
		}
	}
	if (iIdxCount < cardsNum)
	{
		return -1;
	}
	else
	{
		return cards[0];
	}
}

int32 OutCardAi::judgePairProgression(uint8* cards, int32 cardsNum)
{
	if (cardsNum < 3)
	{
		return -1;
	}
	int32 iIdxCount = 0;
	for (; iIdxCount < cardsNum; iIdxCount += 2)
	{
		if ((cards[iIdxCount] & 0x0f) >= 12)
		{
			break;
		}
		else if ((cards[iIdxCount] & 0x0f) != (cards[iIdxCount + 1] & 0x0f))
		{
			break;
		}
		else if (iIdxCount < cardsNum - 2)
		{
			if ((cards[iIdxCount] & 0x0f) + 1 != (cards[iIdxCount + 2] & 0x0f))
			{
				break;
			}
		}
	}
	if (iIdxCount < cardsNum)
	{
		return -1;
	}
	else
	{
		return cards[0];
	}
}

int32 OutCardAi::judgeTripleProgression(uint8* cards, int32 cardsNum)
{
	if (cardsNum < 2)
	{
		return -1;
	}
	int32 iIdxCount = 0;
	for (; iIdxCount < cardsNum; iIdxCount += 3)
	{
		if ((cards[iIdxCount] & 0x0f) >= 12)
		{
			break;
		}
		else if ((cards[iIdxCount] & 0x0f) != (cards[iIdxCount + 2] & 0x0f))
		{
			break;
		}
		else if (iIdxCount < cardsNum - 3)
		{
			if ((cards[iIdxCount] & 0x0f) + 1 != (cards[iIdxCount + 3] & 0x0f))
			{
				break;
			}
		}
	}
	if (iIdxCount < cardsNum)
	{
		return -1;
	}
	else
	{
		return cards[0];
	}
}

int32 OutCardAi::judgeTripleWithOne(uint8* cards, int32 cardsNum)
{
	if (cardsNum != 4)
	{
		return -1;
	}
	if ((cards[0] & 0x0f) == (cards[2] & 0x0f) && (cards[0] & 0x0f) != (cards[3] & 0x0f))
	{
		return cards[0];
	}
	else if ((cards[0] & 0x0f) != (cards[1] & 0x0f) && (cards[1] & 0x0f) == (cards[3] & 0x0f))
	{
		return cards[1];
	}
	else
	{
		return -1;
	}
}

int32 OutCardAi::judgeTripleWithTwo(uint8* cards, int32 cardsNum)
{
	if (cardsNum != 5)
	{
		return -1;
	}
	if ((cards[0] & 0x0f) == (cards[2] & 0x0f) && (cards[0] & 0x0f) != (cards[3] & 0x0f)
		&& (cards[3] & 0x0f) == (cards[4] & 0x0f))
	{
		return cards[0];
	}
	else if ((cards[0] & 0x0f) == (cards[1] & 0x0f) && (cards[0] & 0x0f) != (cards[2] & 0x0f)
		&& (cards[2] & 0x0f) == (cards[4] & 0x0f))
	{
		return cards[2];
	}
	else
	{
		return -1;
	}
}

int32 OutCardAi::judgeFourWithTwo(uint8* cards, int32 cardsNum)
{
	if (cardsNum != 6 && cardsNum != 8)
	{
		return -1;
	}
	if (cardsNum == 6)
	{
		if ((cards[0] & 0x0f) == (cards[3] & 0x0f) && (cards[4] & 0x0f) != (cards[5] & 0x0f))
		{
			return cards[0];
		}
		else if ((cards[1] & 0x0f) == (cards[4] & 0x0f) && (cards[0] & 0x0f) != (cards[5] & 0x0f))
		{
			return cards[1];
		}
		else if ((cards[2] & 0x0f) == (cards[5] & 0x0f) && (cards[0] & 0x0f) != (cards[1] & 0x0f))
		{
			return cards[2];
		}
		else
		{
			return -1;
		}
	}
	else if (cardsNum == 8)
	{
		if ((cards[0] & 0x0f) == (cards[3] & 0x0f) && (cards[4] & 0x0f) == (cards[5] & 0x0f)
			&& (cards[6] & 0x0f) == (cards[7] & 0x0f))
		{
			return cards[0];
		}
		else if ((cards[2] & 0x0f) == (cards[5] & 0x0f) && (cards[0] & 0x0f) == (cards[1] & 0x0f)
			&& (cards[6] & 0x0f) == (cards[7] & 0x0f))
		{
			return cards[2];
		}
		else if ((cards[4] & 0x0f) == (cards[7] & 0x0f) && (cards[0] & 0x0f) == (cards[1] & 0x0f)
			&& (cards[2] & 0x0f) == (cards[3] & 0x0f))
		{
			return cards[4];
		}
		else
		{
			return -1;
		}
	}

	return -1;
}

int32 OutCardAi::judgeAirPlane(uint8* cards, int32 cardsNum)
{
	if ((cardsNum % 4) != 0 && (cardsNum % 5) != 0)
	{
		return -1;
	}

	if (cardsNum % 4 == 0)
	{
		int32	iCount = cardsNum >> 2;
		if ((cards[0] & 0x0f) + 1 == (cards[5] & 0x0f))
		{
			return judgeTripleProgression(cards, cardsNum - iCount);
		}
		else if ((cards[1] & 0x0f) + 1 == (cards[6] & 0x0f))
		{
			return judgeTripleProgression(cards + 1, cardsNum - iCount);
		}
		else if ((cards[2] & 0x0f) + 1 == (cards[7] & 0x0f))
		{
			return judgeTripleProgression(cards + 2, cardsNum - iCount);
		}
		else if ((cards[3] & 0x0f) + 1 == (cards[8] & 0x0f))
		{
			return judgeTripleProgression(cards + 3, cardsNum - iCount);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		int32	iCount = cardsNum / 5;
		int32 iCardIdx = 0;
		int32 iResult = -1;
		while (iCardIdx < cardsNum)
		{
			if ((cards[iCardIdx] & 0x0f) != (cards[iCardIdx + 1] & 0x0f))
			{
				iResult = -1;
				break;
			}
			else if ((cards[iCardIdx] & 0x0f) == (cards[iCardIdx + 2] & 0x0f))
			{
				if (judgeTripleProgression(cards + iCardIdx, (cardsNum - iCount * 3)) >= 0)
				{
					iResult = cards[iCardIdx];
					iCardIdx += (cardsNum - iCount * 3);
				}
				else
				{
					iResult = -1;
					break;
				}
			}
			else
			{
				iCardIdx += 2;
			}
		}
		return iResult;
	}

}
