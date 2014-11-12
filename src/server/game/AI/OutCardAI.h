#ifndef _OUTCARDAI_H
#define _OUTCARDAI_H

#include "Player.h"

class Player;

class OutCardAi
{
public:
	static OutCardAi* instance()
	{
		static OutCardAi instance;
		return &instance;
	}
	void OutCard(Player *player);
	void updateCardsFace(uint8* SelfCard, uint8* OutCard);
	uint32 getCardsNumber(uint8 * cards);

private:
	OutCardAi();
	~OutCardAi();
	
	void resetCards(uint8 * cards);
	void rearrangeCards(uint8* SelfCards, uint8 Card);

	int32 getPlayOutCard(Player * player, CardType *cardType);
	int32 getPlayOutCard(CardType cardType, uint8 recvCard[], uint8 selfCards[], uint8 outCards[], bool bSplitCard);

	CardType getPlayOutCardFirst(Player *player);
	CardType getPlayOutCardFirst(uint8 SelfCards[], uint8 OutCards[]);

	CardType IsSeriesCard(uint8 SelfCards[], uint8 OutCards[]);
	CardType playOutCardsNotByType(uint8 SelfCard[], uint8 OutCards[], int32 CardType, int32 SelfCardsNum);
	CardType playOutMaxCardsByType(uint8 selfCards[], uint8 outCards[], CardType cardType, int32 selfCardsNum);
	int32 getMainCardFromRecv(CardType cardType, uint8* cardsRecv, int32 cardsRecvNum);
	bool bSplitCards(Player * player);
	int32 getPlayOutCardType(uint8 outCards[], CardType* cardType);


	int32 getSingleCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);
	int32 forceGetSingleCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);
	int32 getPairCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);
	int32 forceGetPairCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);

	int32 getTripleCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);
	int32 getBombCard(uint8* SelfCards, uint8* OutCards, int32 CardPos);
	int32 getRocketCard(uint8* SelfCards, uint8* OutCards);
	int32 getSingleProgression(uint8* SelfCards, uint8* OutCards, int32 CardPos, int32 CardsNum);
	int32 getPairProgression(uint8* SelfCards, uint8* OutCards, int32 CardPos, int32 CardsNum);
	int32 getTripleProgression(uint8* SelfCards, uint8* OutCards, int32 CardBegPos, int32 CardsNum);
	int32 getTripleWithOne(uint8* SelfCards, uint8* OutCards, int32 CardBegPos, int32 CardsNum);
	int32 getTripleWithTwo(uint8* SelfCards, uint8* OutCards, int32 CardBegPos, int32 CardsNum);
	int32 getAirPlane(uint8* SelfCards, uint8* OutCards, int32 CardBegPos, int32 CardsNum);

	int judgeSingleProgression(uint8* cards, int32 cardsNum);
	int judgePairProgression(uint8* cards, int32 cardsNum);
	int judgeTripleProgression(uint8* cards, int32 cardsNum);
	int judgeTripleWithOne(uint8* cards, int32 cardsNum);
	int judgeTripleWithTwo(uint8* cards, int32 cardsNum);
	int judgeFourWithTwo(uint8* cards, int32 cardsNum);
	int judgeAirPlane(uint8* cards, int32 cardsNum);
};

#define sOutCardAi OutCardAi::instance()

#endif