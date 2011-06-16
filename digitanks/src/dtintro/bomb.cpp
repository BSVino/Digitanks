#include "bomb.h"

#include <renderer/renderer.h>

REGISTER_ENTITY(CBomb);

NETVAR_TABLE_BEGIN(CBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBomb);
	SAVEDATA_OMIT(m_flExplodeTime);
	SAVEDATA_OMIT(m_hTrailParticles);
	SAVEDATA_OMIT(m_hTarget);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBomb);
INPUTS_TABLE_END();

void CBomb::Precache()
{
	PrecacheParticleSystem(L"shell-trail");
	PrecacheParticleSystem(L"explosion");
	PrecacheSound(L"sound/explosion.wav");
}

void CBomb::Spawn()
{
	BaseClass::Spawn();

	m_flExplodeTime = 0;

	m_hTrailParticles.SetSystem(L"shell-trail", GetOrigin());
	m_hTrailParticles.FollowEntity(this);
	m_hTrailParticles.SetActive(true);
}

void CBomb::Think()
{
	BaseClass::Think();

	if (m_flExplodeTime && GameServer()->GetGameTime() > m_flExplodeTime)
	{
		EmitSound(L"sound/explosion.wav");

		CParticleSystemLibrary::AddInstance(L"explosion", GetOrigin());
		Delete();

		if (m_hTarget != NULL && m_hTarget->GetInput("Dissolve"))
			m_hTarget->CallInput("Dissolve", L"");
	}
}

void CBomb::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
	pContext->SetColor(Color(255, 255, 255, 255));
	pContext->Scale(5, 5, 5);
	pContext->RenderSphere();
}
