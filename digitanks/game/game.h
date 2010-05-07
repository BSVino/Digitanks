#ifndef GAME_H
#define GAME_H

#include <vector>

#include "baseentity.h"

class CGame
{
public:
												CGame();
												~CGame();

public:
	void										Think(float flGameTime);
	void										Simulate();

	virtual void								Think() {};

	virtual void								OnKilled(class CBaseEntity* pEntity) {};
	virtual void								OnDeleted(class CBaseEntity* pEntity) {};

	void										Delete(class CBaseEntity* pEntity);

	static CGame*								GetGame() { return s_pGame; };

protected:
	std::vector<CEntityHandle<CBaseEntity> >	m_ahDeletedEntities;

	static CGame*								s_pGame;

	float										m_flGameTime;
	float										m_flFrameTime;
};

inline class CGame* Game()
{
	return CGame::GetGame();
}

#endif
