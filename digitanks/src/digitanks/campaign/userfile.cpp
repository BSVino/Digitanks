#include "userfile.h"

#include <renderer/renderer.h>
#include <glgui/glgui.h>

#include <digitanksgame.h>
#include <units/digitank.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

REGISTER_ENTITY(CUserFile);

NETVAR_TABLE_BEGIN(CUserFile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CUserFile);
	SAVEDATA_DEFINE_OUTPUT(OnPickup);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iImage);
	SAVEDATA_DEFINE(CSaveData::DATA_STRING, eastl::string, m_sFilename);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPickupTime);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CUserFile);
INPUTS_TABLE_END();

void CUserFile::Spawn()
{
	BaseClass::Spawn();

	m_iImage = ~0;
	m_flPickupTime = 0;
}

EAngle CUserFile::GetRenderAngles() const
{
	float flRotate = fmod(GameServer()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

void CUserFile::ModifyContext(class CRenderingContext* pContext, bool bTransparent) const
{
	// Don't run CDigitanksEntity::ModifyContext, we want to show our user file all the time.
}

void CUserFile::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
	if (!bTransparent)
		return;

	if (m_flPickupTime > 0)
	{
		float flScale = RemapValClamped(GameServer()->GetGameTime(), m_flPickupTime, m_flPickupTime + 1, 1, 0);
		pContext->Scale(flScale, flScale, flScale);
	}

	pContext->SetBlend(BLEND_ADDITIVE);
	pContext->SetBackCulling(false);
	pContext->BindTexture(m_iImage);
	pContext->BeginRenderQuads();

	pContext->TexCoord(0, 1);
	pContext->Vertex(Vector(5, 10, 0));

	pContext->TexCoord(0, 0);
	pContext->Vertex(Vector(5, 0, 0));

	pContext->TexCoord(1, 0);
	pContext->Vertex(Vector(-5, 0, 0));

	pContext->TexCoord(1, 1);
	pContext->Vertex(Vector(-5, 10, 0));

	pContext->EndRender();

	CRenderingContext c(GameServer()->GetRenderer());

	c.SetBackCulling(true);
	c.Scale(0.1f, 0.1f, 0.1f);
	c.Translate(Vector(0, 105, 0));

	tstring sFile = _T("Your file: ");
	sFile += m_sFilename.substr(m_sFilename.find_last_of(_T('/'))+1);
	float flWidth = glgui::CLabel::GetTextWidth(sFile, sFile.length(), _T("text"), 12);
	glgui::CLabel::PaintText3D(sFile, sFile.length(), _T("text"), 12, Vector(0-flWidth/2, 0, 0));

	c.Scale(-1.0f, 1.0f, 1.0f);
	glgui::CLabel::PaintText3D(sFile, sFile.length(), _T("text"), 12, Vector(0-flWidth/2, 0, 0));
}

void CUserFile::Think()
{
	BaseClass::Think();

	if (m_flPickupTime > 0 && GameServer()->GetGameTime() > m_flPickupTime + 1)
		Delete();
}

bool CUserFile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	if (!IsActive())
		return false;

	if (!pOther)
		return false;

	if (!pOther->GetTeam())
		return false;

	if (!pOther->GetTeam()->IsPlayerControlled())
		return false;

	CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);

	if (!pTank)
		return false;

	if (Distance(pTank->GetRealOrigin()) > 20)
		return false;

	return true;
}

void CUserFile::Pickup(CDigitank* pTank)
{
	if (!IsActive())
		return;

	m_flPickupTime = GameServer()->GetGameTime();

	CallOutput("OnPickup");

	DigitanksWindow()->GetHUD()->ShowFileRescue(m_sFilename);
}

void CUserFile::SetFile(const tstring& sFile)
{
	m_sFilename = sFile;
	m_iImage = CTextureLibrary::AddTextureID(m_sFilename);
}
