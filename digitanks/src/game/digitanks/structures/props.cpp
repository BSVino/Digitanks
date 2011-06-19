#include "props.h"

#include <renderer/renderer.h>
#include <models/models.h>
#include <shaders/shaders.h>

#include <digitanks/digitanksgame.h>

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
	PrecacheModel(_T("models/props/prop01.obj", true);
	PrecacheModel(_T("models/props/prop02.obj", true);
	PrecacheModel(_T("models/props/prop03.obj", true);
	PrecacheModel(_T("models/props/prop04.obj", true);
	PrecacheModel(_T("models/props/prop05.obj", true);
}

void CStaticProp::Spawn()
{
	SetCollisionGroup(CG_PROP);

	m_bAdditive = false;
	m_bDepthMask = true;
	m_bBackCulling = true;

	m_bSwap = false;

	m_bUseRaytracedCollision = true;
}

void CStaticProp::ModifyContext(CRenderingContext* pContext, bool bTransparent) const
{
	pContext->SetColorSwap(m_clrSwap);

	BaseClass::ModifyContext(pContext, bTransparent);
}

void CStaticProp::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
	CModel* pModel = CModelLibrary::Get()->GetModel(GetModel());

	if (!pModel)
		return;

	if (bTransparent)
		return;

	CRenderer* pRenderer = GameServer()->GetRenderer();
	CRenderingContext c(pRenderer);

	if (pRenderer->ShouldUseShaders())
	{
		c.UseProgram(CShaderLibrary::GetPropProgram());
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
			c.SetUniform("vecTankOrigin", pCurrentTank->GetOrigin());
		}

		c.SetUniform("bColorSwapInAlpha", true);

		Color clrSwap = m_clrSwap;
		c.SetUniform("vecColorSwap", clrSwap);

		c.RenderCallList(pModel->m_iCallList);
	}
	else
	{
		Color clrSwap = m_clrSwap;
		clrSwap.SetAlpha((int)(GetVisibility()*255));
		c.SetColor(clrSwap);

		c.RenderCallList(pModel->m_iCallList);
	}

	if (pRenderer->ShouldUseShaders())
		pRenderer->ClearProgram();
}
