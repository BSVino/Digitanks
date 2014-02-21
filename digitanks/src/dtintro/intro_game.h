#ifndef DT_INTRO_GAME_H
#define DT_INTRO_GAME_H

#include <game/entities/game.h>

class CIntroGame : public CGame
{
	REGISTER_ENTITY_CLASS(CIntroGame, CGame);

public:
	void  SetupGame(tstring sType);
	void  SetupIntro();

	virtual void	Precache();

	virtual void	Think();
};

#endif
