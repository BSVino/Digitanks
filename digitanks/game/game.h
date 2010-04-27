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
	void										Think();

	virtual void								OnKilled(class CBaseEntity* pEntity) {};
	virtual void								OnDeleted(class CBaseEntity* pEntity) {};

	void										Delete(class CBaseEntity* pEntity);

	static CGame*								GetGame() { return s_pGame; };

protected:
	std::vector<CEntityHandle<CBaseEntity> >	m_ahDeletedEntities;

	static CGame*								s_pGame;
};

inline class CGame* Game()
{
	return CGame::GetGame();
}

#endif
