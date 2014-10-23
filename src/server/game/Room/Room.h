#ifndef _ROOM_H
#define _ROOM_H

#include "Timer.h"

class Room
{
public:
	Room(uint32 id, uint32 basic_score);
	~Room();
	void Update(const uint32 t_diff);
private:
	uint32 _id;
	uint32 _basic_score;
};

#endif