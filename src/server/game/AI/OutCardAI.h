#ifndef _OUTCARDAI_H
#define _OUTCARDAI_H

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
};

#define sOutCardAi OutCardAi::instance()

#endif