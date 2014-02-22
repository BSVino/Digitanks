#include "intro_camera.h"

#include <tinker/application.h>
#include <game/gameserver.h>
#include <tinker/cvar.h>
#include <game/cameramanager.h>

REGISTER_ENTITY(CIntroCamera);

NETVAR_TABLE_BEGIN(CIntroCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroCamera);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroCamera);
INPUTS_TABLE_END();

void CIntroCamera::Spawn()
{
	BaseClass::Spawn();

	m_bOrtho = true;

	m_flOrthoHeight = (float)CApplication::Get()->GetWindowHeight()/2;

	m_flZoomIntoHole = 0;
}

void ZoomIntoHole(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	static_cast<CIntroCamera*>(GameServer()->GetCameraManager()->GetActiveCamera())->ZoomIntoHole();
}

CCommand zoomintohole("zoomintohole", ::ZoomIntoHole);

void CIntroCamera::CameraThink()
{
	if (m_flZoomIntoHole)
	{
		float flWidth = (float)CApplication::Get()->GetWindowWidth();
		float flHeight = (float)CApplication::Get()->GetWindowHeight();

		float flLerp = Bias((float)RemapValClamped(GameServer()->GetGameTime(), m_flZoomIntoHole, m_flZoomIntoHole+1.5f, 0.0, 1.0), 0.2f);

		SetGlobalOrigin(LerpValue(TVector(500, 0, 0), TVector(500, 350, 50), flLerp));

		m_flOrthoHeight = LerpValue((float)CApplication::Get()->GetWindowHeight()/2, (float)CApplication::Get()->GetWindowHeight()/200, flLerp);

		// Kind of a lame place to put it but I don't care!
		if (GameServer()->GetGameTime() - m_flZoomIntoHole > 1.5f)
			exit(0);
	}
	else
	{
		SetGlobalOrigin(TVector(500, 0, 0));
		SetGlobalAngles(EAngle(0, 180, 0));
	}
}

void CIntroCamera::ZoomIntoHole()
{
	m_flZoomIntoHole = GameServer()->GetGameTime();
}
