#include "screen.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>
#include <renderer/particles.h>
#include <models/models.h>

REGISTER_ENTITY(CScreen);

NETVAR_TABLE_BEGIN(CScreen);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CScreen);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CScreen);
INPUTS_TABLE_END();

void CScreen::Precache()
{
	PrecacheModel("models/intro/screen.toy");
	PrecacheParticleSystem("intro-explosion-fragments");
	PrecacheSound("sound/tank-damage.wav");
	PrecacheSound("sound/helper-speech.wav");	// For the UI
}

void CScreen::Spawn()
{
	SetModel("models/intro/screen.toy");
}

void CScreen::ModifyContext(class CRenderingContext* pContext) const
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	pContext->Scale((flWidth+flHeight)/2, flHeight, flWidth);
}

void CScreen::OnRender(class CGameRenderingContext* pContext) const
{
}

void CScreen::SetScreenshot(size_t iScreenshot)
{
	size_t iModel = CModelLibrary::Get()->FindModel("models/intro/screen.toy");
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);
//	pModel->m_aiTextures[0] = iScreenshot;
//	pModel->m_iCallListTexture = iScreenshot;

	iModel = CModelLibrary::Get()->FindModel("models/intro/screen-fragment.toy");
	pModel = CModelLibrary::Get()->GetModel(iModel);
//	pModel->m_aiTextures[0] = iScreenshot;
//	pModel->m_iCallListTexture = iScreenshot;

	EmitSound("sound/tank-damage.wav");
}
