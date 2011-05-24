#include "intro_game.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <game/level.h>

#include "intro_camera.h"
#include "intro_renderer.h"

CGame* CreateGame()
{
	return GameServer()->Create<CIntroGame>("CIntroGame");
}

CRenderer* CreateRenderer()
{
	return new CIntroRenderer();
}

CCamera* CreateCamera()
{
	CCamera* pCamera = new CIntroCamera();
	return pCamera;
}

CLevel* CreateLevel()
{
	return new CLevel();
}

REGISTER_ENTITY(CIntroGame);

NETVAR_TABLE_BEGIN(CIntroGame);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroGame);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroGame);
INPUTS_TABLE_END();

void CIntroGame::Think()
{
	BaseClass::Think();

	if (GameServer()->GetGameTime() > 10)
		exit(0);
}
