
#ifndef __WORLD_H
#define __WORLD_H
/// The World

class World
{
public:
	static World* instance()
	{
		static World instance;
		return &instance;
	}
private:
	World();
	~World();
};

#define sWorld World::instance()

#endif