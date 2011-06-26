#include "screen.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
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
	PrecacheModel(_T("models/intro/screen.obj"));
	PrecacheParticleSystem(_T("intro-explosion-fragments"));
	PrecacheSound(_T("sound/tank-damage.wav"));
	PrecacheSound(_T("sound/helper-speech.wav"));	// For the UI
}

void CScreen::Spawn()
{
	SetModel(_T("models/intro/screen.obj"));
}

void CScreen::ModifyContext(class CRenderingContext* pContext, bool bTransparent) const
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	pContext->Scale((flWidth+flHeight)/2, flHeight, flWidth);
}

void CScreen::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
}

void CScreen::SetScreenshot(size_t iScreenshot)
{
	size_t iModel = CModelLibrary::Get()->FindModel(_T("models/intro/screen.obj"));
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);
	pModel->m_aiTextures[0] = iScreenshot;
	pModel->m_iCallListTexture = iScreenshot;

	iModel = CModelLibrary::Get()->FindModel(_T("models/intro/screen-fragment.obj"));
	pModel = CModelLibrary::Get()->GetModel(iModel);
	pModel->m_aiTextures[0] = iScreenshot;
	pModel->m_iCallListTexture = iScreenshot;

	EmitSound(_T("sound/tank-damage.wav"));
}
