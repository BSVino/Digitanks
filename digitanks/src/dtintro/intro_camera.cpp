#include "intro_camera.h"

#include <tinker/application.h>

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
}

void CIntroCamera::CameraThink()
{
	SetGlobalOrigin(TVector(500, 0, 0));
	SetGlobalAngles(EAngle(0, 180, 0));
}
