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

private:
	OutCardAi();
	~OutCardAi();
	
	uint32 getCardsNumber(uint8 * cards);
	void resetCards(uint8 * cards);
	void OutCardAi::rearrangeCards(uint8* SelfCards, uint8 Card);
};

#define sOutCardAi OutCardAi::instance()

#endif