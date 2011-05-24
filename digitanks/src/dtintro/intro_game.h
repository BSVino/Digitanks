#ifndef DT_INTRO_GAME_H
#define DT_INTRO_GAME_H

#include <game/game.h>

class CIntroGame : public CGame
{
	REGISTER_ENTITY_CLASS(CIntroGame, CGame);

public:
	virtual void	Think();
};

#endif
