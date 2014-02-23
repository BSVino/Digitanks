#include "props.h"

#include <renderer/game_renderer.h>
#include <models/models.h>
#include <renderer/shaders.h>
#include <renderer/game_renderingcontext.h>
#include <game/gameserver.h>

#include <digitanksgame.h>

REGISTER_ENTITY(CStaticProp);

NETVAR_TABLE_BEGIN(CStaticProp);
	NETVAR_DEFINE(bool, m_bAdditive);
	NETVAR_DEFINE(bool, m_bDepthMask);
	NETVAR_DEFINE(bool, m_bBackCulling);
	NETVAR_DEFINE(bool, m_bSwap);
	NETVAR_DEFINE(Color, m_clrSwap);
	NETVAR_DEFINE(bool, m_bUseRaytracedCollision);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStaticProp);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bAdditive);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bDepthMask);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bBackCulling);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Color, m_clrSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bUseRaytracedCollision);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CStaticProp);
INPUTS_TABLE_END();

void CStaticProp::Precache()
{
	PrecacheModel("models/props/prop01.toy");
	PrecacheModel("models/props/prop02.toy");
	PrecacheModel("models/props/prop03.toy");
	PrecacheModel("models/props/prop04.toy");
	PrecacheModel("models/props/prop05.toy");
}

void CStaticProp::Spawn()
{
	m_bAdditive = false;
	m_bDepthMask = true;
	m_bBackCulling = true;

	m_bSwap = false;

	m_bUseRaytracedCollision = true;
}

void CStaticProp::ModifyContext(CRenderingContext* pContext) const
{
	pContext->SetUniform("bColorSwapInAlpha", true);
	pContext->SetUniform("vecColorSwap", m_clrSwap);

	BaseClass::ModifyContext(pContext);
}

void CStaticProp::OnRender(class CGameRenderingContext* pContext) const
{
	if (!GetModel())
		return;

	CGameRenderer* pRenderer = GameServer()->GetRenderer();

	if (pRenderer->IsRenderingTransparent())
		return;

	CGameRenderingContext c(pRenderer, true);

	c.UseProgram("prop");
	c.SetUniform("bDiffuse", true);
	c.SetUniform("iDiffuse", 0);
	c.SetUniform("flAlpha", GetVisibility());

	bool bModeMove = DigitanksGame()->GetControlMode() == MODE_MOVE;
	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();
	if (!pCurrentTank)
		bModeMove = false;

	c.SetUniform("bMovement", bModeMove);

	if (bModeMove)
	{
		c.SetUniform("flMoveDistance", pCurrentTank->GetRemainingMovementDistance());
		c.SetUniform("vecTankOrigin", pCurrentTank->GetGlobalOrigin());
	}

	c.SetUniform("bColorSwapInAlpha", true);

	Color clrSwap = m_clrSwap;
	c.SetUniform("vecColorSwap", clrSwap);

	c.RenderModel(GetModelID());
}
