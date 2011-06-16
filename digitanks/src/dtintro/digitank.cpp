#include "digitank.h"

#include <mtrand.h>

#include <renderer/renderer.h>
#include <models/models.h>

REGISTER_ENTITY(CDigitank);

NETVAR_TABLE_BEGIN(CDigitank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitank);
	SAVEDATA_OMIT(m_vecNextAim);
	SAVEDATA_OMIT(m_flNextFire);
	SAVEDATA_OMIT(m_hTarget);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitank);
	INPUT_DEFINE(FireAt);
INPUTS_TABLE_END();

void CDigitank::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj");
	PrecacheModel(L"models/digitanks/digitank-turret.obj");
}

void CDigitank::Spawn()
{
	SetModel(L"models/digitanks/digitank-body.obj");
	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");

	m_flNextFire = 0;
}

void CDigitank::Think()
{
	BaseClass::Think();

	if (m_flNextFire && GameServer()->GetGameTime() > m_flNextFire)
	{
		FireBomb(m_vecNextAim, m_hTarget);

		m_flNextFire = 0;
	}
}

void CDigitank::FireAt(const eastl::vector<eastl::string16>& sArgs)
{
	if (sArgs.size() == 0)
		return;

	m_flNextFire = GameServer()->GetGameTime() + 0.5f;

	eastl::vector<CBaseEntity*> apEntities;
	CBaseEntity::FindEntitiesByName(convertstring<char16_t, char>(sArgs[0]), apEntities);

	if (apEntities.size() == 0)
		return;

	m_hTarget = apEntities[0];
	m_vecNextAim = apEntities[0]->GetOrigin();

	FaceTurret(VectorAngles(m_vecNextAim-GetOrigin()).y - GetAngles().y);
}
