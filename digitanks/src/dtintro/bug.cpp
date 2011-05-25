#include "bug.h"

#include <mtrand.h>

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <models/models.h>

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
INPUTS_TABLE_END();

void CBug::Precache()
{
	PrecacheModel(L"models/digitanks/gridbug.obj");
	PrecacheModel(L"models/digitanks/gridbug-turret.obj");
}

void CBug::Spawn()
{
	SetModel(L"models/digitanks/gridbug.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/gridbug-turret.obj");

	m_bFiringRandomly = false;
	m_flNextAim = m_flNextFire = 0;
}

void CBug::Think()
{
	if (m_bFiringRandomly && m_flNextAim && GameServer()->GetGameTime() > m_flNextAim)
	{
		float flWidth = (float)CApplication::Get()->GetWindowWidth();
		float flHeight = (float)CApplication::Get()->GetWindowHeight();

		do
		{
			m_vecNextAim = Vector(RandomFloat(50, 200), RandomFloat(-flHeight/2, flHeight/2), RandomFloat(-flWidth/2, flWidth/2));
		} while ((m_vecNextAim-GetOrigin()).Length2D() < 200);

		FaceTurret(VectorAngles(m_vecNextAim-GetOrigin()).y - GetAngles().y);

		m_flNextAim = 0;
		m_flNextFire = GameServer()->GetGameTime() + RandomFloat(0.5f, 1.0f);
	}

	if (m_bFiringRandomly && m_flNextFire && GameServer()->GetGameTime() > m_flNextFire)
	{
		FireBomb(m_vecNextAim);

		m_flNextFire = 0;
		m_flNextAim = GameServer()->GetGameTime() + RandomFloat(1.0f, 1.5f);
	}
}

void CBug::FireRandomly(const eastl::vector<eastl::string16>& sArgs)
{
	m_bFiringRandomly = true;

	m_flNextAim = GameServer()->GetGameTime() + RandomFloat(0, 0.5f);
}
