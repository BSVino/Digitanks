#include "screen.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
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
	PrecacheModel(L"models/intro/screen.obj");
}

void CScreen::Spawn()
{
	SetModel(L"models/intro/screen.obj");
}

void CScreen::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	pContext->Scale((flWidth+flHeight)/2, flHeight, flWidth);
}

void CScreen::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
}

void CScreen::SetScreenshot(size_t iScreenshot)
{
	size_t iModel = CModelLibrary::Get()->FindModel(L"models/intro/screen.obj");
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);
	pModel->m_aiTextures[0] = iScreenshot;
	pModel->m_iCallListTexture = iScreenshot;
}
