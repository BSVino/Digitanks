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

	pContext->Scale((flWidth + flHeight)/2, flWidth, flHeight);
	pContext->BindTexture(m_iScreenshot);
}

bool CScreen::ModifyShader(class CRenderingContext* pContext) const
{
#ifdef _WIN32
	// Windows gives the screenshot data back as BGR.
	// Instead of switching the red and blue channels in the screenshot data,
	// I'll just load a different shader.
	pContext->UseProgram("screen_win32");
#endif

	pContext->BindTexture(m_iScreenshot);
	pContext->SetUniform("bDiffuse", true);

	return true;
}

void CScreen::OnRender(class CGameRenderingContext* pContext) const
{
}

void CScreen::SetScreenshot(size_t iScreenshot)
{
	m_iScreenshot = iScreenshot;

	EmitSound("sound/tank-damage.wav");
}
