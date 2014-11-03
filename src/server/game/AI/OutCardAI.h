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
private:
	OutCardAi();
	~OutCardAi();
	
	uint32 getCardsNumber(uint8 * cards);
	void resetCards(uint8 * cards);
};

#define sOutCardAi OutCardAi::instance()

#endif