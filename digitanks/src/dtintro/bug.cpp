#include "bug.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <models/models.h>

REGISTER_ENTITY(CBug);

NETVAR_TABLE_BEGIN(CBug);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBug);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBug);
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
}
