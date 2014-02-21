#include "bug.h"

#include <mtrand.h>

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <models/models.h>

#include "../digitanks/dissolver.h"

REGISTER_ENTITY(CBug);

NETVAR_TABLE_BEGIN(CBug);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBug);
	SAVEDATA_OMIT(m_bFiringRandomly);
	SAVEDATA_OMIT(m_vecNextAim);
	SAVEDATA_OMIT(m_flNextAim);
	SAVEDATA_OMIT(m_flNextFire);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBug);
	INPUT_DEFINE(FireRandomly);
	INPUT_DEFINE(Dissolve);
INPUTS_TABLE_END();

void CBug::Precache()
{
	PrecacheModel("models/digitanks/gridbug.toy");
	PrecacheModel("models/digitanks/gridbug-turret.toy");

	// For the bombs.
	PrecacheParticleSystem("shell-trail");
	PrecacheParticleSystem("explosion");
	PrecacheSound("sound/explosion.wav");
}

void CBug::Spawn()
{
	SetModel("models/digitanks/gridbug.toy");
	m_iTurretModel = CModelLibrary::Get()->FindModel("models/digitanks/gridbug-turret.toy");

	m_bFiringRandomly = false;
	m_flNextAim = m_flNextFire = 0;
}

void CBug::Think()
{
	BaseClass::Think();

	if (m_bFiringRandomly && m_flNextAim && GameServer()->GetGameTime() > m_flNextAim)
	{
		float flWidth = (float)CApplication::Get()->GetWindowWidth();
		float flHeight = (float)CApplication::Get()->GetWindowHeight();

		do
		{
			m_vecNextAim = Vector(RandomFloat(50, 200), RandomFloat(-flWidth/2, flWidth/2), RandomFloat(-flHeight/2, flHeight/2));
		} while ((m_vecNextAim-GetGlobalOrigin()).Length2D() < 200);

		FaceTurret(VectorAngles(m_vecNextAim-GetGlobalOrigin()).y - GetGlobalAngles().y);

		m_flNextAim = 0;
		m_flNextFire = GameServer()->GetGameTime() + RandomFloat(0.5f, 1.0f);
	}

	if (m_bFiringRandomly && m_flNextFire && GameServer()->GetGameTime() > m_flNextFire)
	{
		FireBomb(m_vecNextAim);

		m_flNextFire = 0;
		m_flNextAim = GameServer()->GetGameTime() + RandomFloat(2.0f, 2.5f);
	}
}

void CBug::FireRandomly(const tvector<tstring>& sArgs)
{
	m_bFiringRandomly = true;

	m_flNextAim = GameServer()->GetGameTime() + RandomFloat(0, 0.5f);
}

void CBug::Dissolve(const tvector<tstring>& sArgs)
{
	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	float flScale = 15*flWidth/flHeight;

	Vector vecScale(flScale, flScale, flScale);

	Color clrTeam = GetTeam()->GetColor();
	CModelDissolver::AddModel(this, &clrTeam, &vecScale);

	CModel* pModel = CModelLibrary::Get()->GetModel(m_iTurretModel);

	Matrix4x4 mTransform;
	mTransform.SetTranslation(GetRenderOrigin() + Vector(-0.0f, 0, 0.810368f));
	mTransform.SetAngles(GetRenderAngles() - EAngle(0, m_flCurrentTurretYaw, 0));

	Matrix4x4 mScale;
	mScale.SetScale(vecScale);
	mTransform = mTransform * mScale;

	CModelDissolver::Get()->AddScene(pModel, mTransform, &clrTeam);

	Delete();
}
