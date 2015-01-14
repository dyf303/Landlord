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
	/// ������ǵ���
	if (player->getLandlordId() == player->getid())
	{
		/// ֱ�ӳ���
		return getPlayOutCardFirst(player->_cards, player->_outCards);
	}

	/// �Լ������ǲ���һ����(��������)
	if (-1 != IsSeriesCard(player->_cards, player->_outCards))
	{
		/// ֱ�ӳ���
	//	memset(pkQueueDeskDataNode->pcCardsOutAi, CARD_TERMINATE, sizeof(pkQueueDeskDataNode->pcCardsOutAi));
		return getPlayOutCardFirst(player->_cards, player->_outCards);
	}

	/// ����¼��ǶԼң���öԼ����е�ʣ������(С�ı����߶Լ�)
	if (player->getPlayerGameType() != player->_right->getPlayerGameType())
	{
		/// ��öԼ�����ʣ������
		int32 CardsEnemyNum = getCardsNumber(player->_right->_cards);

		/// С��5�ţ�����ҪС��(��ѶԼҸ�������)
		if (5 > CardsEnemyNum)
		{
			/// ����Լ�����ʣ������
			int32 CardsSelfNum = getCardsNumber(player->_cards);

			/// �Լ�����ֻ��1��
			if (1 == CardsEnemyNum)
			{
				/// ����֧�������
				if (1 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_SINGLE, CardsSelfNum);
				}
			}
			else if (2 == CardsEnemyNum) /// ����2��
			{
				/// �������������
				if (2 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_PAIR, CardsSelfNum);
				}
			}
			else if (3 == CardsEnemyNum) /// ����3��
			{
				/// ����֧�������
				if (3 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_TRPILE, CardsSelfNum);
				}
			}
			else if (4 == CardsEnemyNum) /// ����4��
			{
				/// ������һ�������
				if (4 <= CardsSelfNum)
				{
					return playOutCardsNotByType(player->_cards, player->_outCards, CARD_TYPE_TRIPLE_ONE, CardsSelfNum);
				}
			}
		}
	}

	/// ����ҶԷ��Լ�����ʣ�������(���������Լ�)
	/// ����Լ��������ϵ�λ��
	Player *their = (player->getPlayerGameType() == player->_left->getPlayerGameType() ? player->_left : player->_right);

	/// ��öԷ��Լ�����ʣ������
	int32 iCardsSameFamilyNum = getCardsNumber(their->_cards);

	/// ���ڵ���5�ţ�ֱ�ӳ���
	/// С��5�ţ����Խ�������
	if (5 > iCardsSameFamilyNum)
	{
		/// ����1��
		if (1 == iCardsSameFamilyNum)
		{
			/// ����֧
			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
		else if (2 == iCardsSameFamilyNum) /// ����2��
		{
			/// �����ӣ�û�ж��ӳ���֧
			if (-1 != getPairCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_PAIR;
			}

			if (-1 != getSingleCard(player->_cards, player->_outCards, 0))
			{
				return CARD_TYPE_SINGLE;
			}
		}
		else if (3 == iCardsSameFamilyNum) /// ����3��
		{
			/// ����֧��û����֧�����ӣ�û�ж��ӳ���֧
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
		else if (4 == iCardsSameFamilyNum) /// ����4��
		{
			/// ������һ��û����֧�������ӣ�û�ж��ӳ���֧
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
	/// �¼��ǶԼ�
	if (player->getPlayerGameType() != nextPlayer->getPlayerGameType())
	{
		/// �¼�����ʣ������
		int32 iUserNextCardsNum = getCardsNumber(nextPlayer->_cards);

		/// ��ǰ��������������
		int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
		/// ��ǰ�����û���ʣ�������
		int32 iCardsCurNum = getCardsNumber(player->_curOutCardsPlayer->_cards);

		/// ��ǰ�ҵ����������
		int32 iCardsSelfNum = getCardsNumber(player->_cards);

		/// �����ǰ�������͵��������ʣ���������(��Ҫ�����ƣ�����߶Լ�)
		if (iUserNextCardsNum == iCardsRecvNum)
		{
			/// ��ǰ�����ǵ�֧
			if (1 == iCardsRecvNum || 2 == iCardsRecvNum || 3 == iCardsRecvNum)
			{

				/// ��ǰ�Ѿ�����Ƶ��û����Լ�,�������е���С��3��
				if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType()
					&& iCardsCurNum < 3)
				{
					memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
				}
				else if (CARD_TYPE_PASS != playOutMaxCardsByType(player->_cards, player->_outCards, player->_curOutCardType, iCardsSelfNum))
				{
					/// ��ǰ�յ����Ƶ�������Ϣ
					int32 iMainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum) & 0x0f);

					/// ��ǰ�Լ�������Ƶ�������Ϣ
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

	/// ��ñ�����Ҫ���������Ϣ
	memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));

	bool bSplitCard = bSplitCards(player);//�Ƿ�Ҫ����

	int32 iMainCardsIdx = getPlayOutCard(player->_curOutCardType, player->_curOutCards
		, player->_cards, player->_outCards, bSplitCard);

	if (CARD_TERMINATE == player->_outCards[0])
	{
		memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
		*cardType = CARD_TYPE_PASS;
		return 1;
	}

	getPlayOutCardType(player->_outCards, cardType);
	/// �������һ����()
	/// ����Լ�����ʣ������
	int32 iCardsSelfNum = getCardsNumber(player->_cards);

	/// ���������ȫ�����Դ�ס(һ�ֳ��꣬Ӯ��)
	if (iCardsSelfNum == getCardsNumber(player->_outCards))
	{
		/// �������ƣ�Ӯ��
	}
	else
	{
		/// ��ǰ�Ѿ�����Ƶ��û����Լ�
		if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType())
		{
			//��ǰ�����û����е�����
			int32 iCardCurNumber = getCardsNumber(player->_curOutCardsPlayer->_cards);
			/// ��ǰ��������������
			int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);

			//����ԼҴ�����ƺ������е�����һ����������ֻʣ1,2���ƣ��Ҳ�����
			if (iCardCurNumber == iCardsRecvNum || iCardCurNumber == 1 || iCardCurNumber == 2)
			{
				memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
				*cardType = CARD_TYPE_PASS;
			}
			/// ����ҵ�������ը��
			if (CARD_TYPE_ROCKET == *cardType || CARD_TYPE_BOMB == *cardType)
			{
				// 			/// ����Լ�����ʣ������
				// 			int32 iCardsSelfNum = getCardsNum(pkQueueDeskDataNode->pcCardsTotal[iQueueUserIdx]);
				if (4 == iCardsSelfNum || 2 == iCardsSelfNum)//???�����ǻ����������������
				{
					/// �������ƣ�Ӯ��
				}
				else
				{
					/// ����ǻ��(˫��)
					if (CARD_TYPE_ROCKET == *cardType)
					{
						if (3 == iCardsSelfNum)
						{
							/// �������ƣ�Ӯ��
						}
						else
						{
							/// �Ҳ����� pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
					}
					else /// �Ҳ����� pass
					{
						memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
						*cardType = CARD_TYPE_PASS;
					}
				}
			}
			else /// ����ҵ����Ʋ���ը��
			{
				/// ����ҵ�������Ϣ
				int32 iMainCardsNumSelf = (player->_cards[iMainCardsIdx] & 0x0f);

				/// ��õ�ǰ�����������Ϣ
				int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
				int32 iMainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum) & 0x0f);

				// 			/// ����Լ�����ʣ������
				// 			int32 iCardsSelfNum = getCardsNum(pkQueueDeskDataNode->pcCardsTotal[iQueueUserIdx]);

				/// ����Լ������������ʹ����������һ��
				if (iCardsRecvNum == iCardsSelfNum)
				{
					/// �������ƣ�Ӯ��
				}
				else
				{
					if (CARD_TYPE_SINGLE == player->_curOutCardType)/// ��֧
					{
						///  ��ǰ�������� >= A
						if (11 <= iMainCardNumRecv)
						{
							/// �Ҳ����� pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else ///  ��ǰ�������� < A
						{
							/// �ҵ����� <= A
							if (11 >= iMainCardsNumSelf)
							{
								/// ��������
							}
							else /// �ҵ����� > A
							{
								/// �Ҳ����� pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else if (CARD_TYPE_PAIR == player->_curOutCardType) /// ����
					{
						/// ��ǰ�������� >= K
						if (10 <= iMainCardNumRecv)
						{
							/// �Ҳ����� pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else /// ��ǰ�������� < K
						{
							/// �ҵ����� < K
							if (10 > iMainCardsNumSelf)
							{
								/// ��������
							}
							else /// �ҵ����� >= K
							{
								/// �Ҳ����� pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else if (CARD_TYPE_TRPILE == player->_curOutCardType
						|| CARD_TYPE_TRIPLE_ONE == player->_curOutCardType
						|| CARD_TYPE_TRIPLE_TWO == player->_curOutCardType) /// ��֧
					{
						/// ��ǰ�������� >= K
						if (10 <= iMainCardNumRecv)
						{
							/// �Ҳ����� pass
							memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
							*cardType = CARD_TYPE_PASS;
						}
						else /// ��ǰ�������� < K
						{
							/// �ҵ����� < J
							if (8 > iMainCardsNumSelf)
							{
								/// ��������
							}
							else /// �ҵ����� >= J
							{
								/// �Ҳ����� pass
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
					}
					else /// ��ǰ������������
					{
						/// �Ҳ����� pass
						memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
						*cardType = CARD_TYPE_PASS;
					}
				}
			}
		}
		else /// ��ǰ�Ѿ�����Ƶ��û��ǶԼ�
		{
			///DEBUG(4, "User PlayOutCard  303......\n");
			/// ��õ�ǰ�����������Ϣ
			int32 iCardsRecvNum = getCardsNumber(player->_curOutCards);
			int32 iMainCardNumRecv = getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, iCardsRecvNum);

			/// ը��
			if (CARD_TYPE_ROCKET == player->_curOutCardType
				|| CARD_TYPE_BOMB == player->_curOutCardType)
			{
				/// ����������(����ը����Ҫ��)
			}
			else /// ��ǰ������������
			{
				/// ����ҵ�������ը��
				if (CARD_TYPE_ROCKET == *cardType || CARD_TYPE_BOMB == *cardType)
				{
					/// ����Լ����о�ʣ��1��2���ƣ��Ҿͳ���(��ֹ�Լ�ʣһ�����ߵ�)
					int32 iUserCardsNumEnemy = getCardsNumber(player->_curOutCardsPlayer->_cards);
					if (1 == iUserCardsNumEnemy || 2 == iUserCardsNumEnemy)
					{
						/// ����������(ը����Ҫ��)
					}
					else
					{
						/// ������¼����Լ�
						if (player->getPlayerGameType() == player->_right->getPlayerGameType())
						{
							/// ����¼������Ƶĸ���
							int32 iCardsSameFamilyNum = getCardsNumber(player->_right->_cards);

							/// ������¼ҵ������������ڵ�ǰ���������,(���ܴ��Ҫ��)
							if (iCardsSameFamilyNum < iCardsRecvNum)
							{
								/// ����������(ը����Ҫ��)
							}
							else /// �Ҳ����� pass
							{
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
						}
						else /// ������¼��ǶԼ�
						{
							if (iUserCardsNumEnemy > 10)///����Լҵ��ƴ���10�ţ�������
							{
								memset(player->_outCards, CARD_TERMINATE, sizeof(player->_outCards));
								*cardType = CARD_TYPE_PASS;
							}
							/// ����������(ը����Ҫ��)
						}
					}
				}
				else /// ����ҵ����Ʋ���ը��
				{
					////DEBUG(4, "User PlayOutCard  306......\n");
					/// ����������
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
	int	iResult = -1;///>=0 ��ʾ�õ����Ƶ���ʼ����

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

			if (iResult == -1 && bSplitCard)//���û���ҳ����ţ��ҿ��Բ��ƣ����һ������
				iResult = forceGetSingleCard(selfCards, outCards, iSelfIdx);

		}
			break;
		case CARD_TYPE_PAIR:
		{
			iResult = getPairCard(selfCards, outCards, iSelfIdx);

			 if (iResult == -1 && bSplitCard)//���û���ҳ����ӣ��ҿ��Բ��ƣ����һ������
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
	//�����û����Լң�������
	if (player->getPlayerGameType() == player->_curOutCardsPlayer->getPlayerGameType())
	{
		return false;
	}

	/// ��ǰ��������������
	int cardsRecvNum = getCardsNumber(player->_curOutCards);
	int mainCardNumRecv = (getMainCardFromRecv(player->_curOutCardType, player->_curOutCards, cardsRecvNum) & 0x0f);
	Player * playerNext = player->_right;

	//����Է����Ƶ�����С��10�����¼����Լң�Ҳ������
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

	//ը�����ܲ�ɵ���
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

	//������ܲ�ɵ���
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
		///������
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
		///�������е���
	}
}

int32 OutCardAi::forceGetPairCard(uint8* selfCards, uint8* outCards, int32 iCardPos)
{
	int32 iSelfIdx = iCardPos;
	while (selfCards[iSelfIdx + 3] != CARD_TERMINATE)
	{
		//ը�����ܲ������
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
		///������
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
		///�������е���
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
		return -1;///������
	}
	else
	{
		outCards[0] = selfCards[SelfIdx];
		outCards[1] = selfCards[SelfIdx + 1];
		outCards[2] = selfCards[SelfIdx + 2];
		outCards[3] = selfCards[SelfIdx + 3];
		return	 SelfIdx;
		///�������е���
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
		return -1;///������
	}
	else
	{
		int32 iIdx = 0;
		for (; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
		///�������е���
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
		///�������е���
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
		return -1;///������
	}
	else
	{
		int32 iIdx = 0;
		for (; iIdx < CardsNum; iIdx++)
		{
			OutCardss[iIdx] = pcTmpCards[iIdx];
		}
		return	 SelfIdx;
		///�������е���
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
	///�������е���
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
	///�������е���
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
