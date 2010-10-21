#include "menumarcher.h"

#include <sstream>

#include <maths.h>
#include <mtrand.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <glgui/glgui.h>

#include "digitanksgame.h"

NETVAR_TABLE_BEGIN(CMenuMarcher);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMenuMarcher);
SAVEDATA_TABLE_END();

CMenuMarcher::~CMenuMarcher()
{
	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);
}

void CMenuMarcher::Precache()
{
	PrecacheModel(L"models/digitanks/digitank-body.obj", true);
	PrecacheModel(L"models/digitanks/digitank-turret.obj", true);
	PrecacheParticleSystem(L"tank-hover");
}

void CMenuMarcher::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/digitanks/digitank-body.obj");

	m_iTurretModel = CModelLibrary::Get()->FindModel(L"models/digitanks/digitank-turret.obj");

	m_iHoverParticles = ~0;

	m_flBobOffset = RandomFloat(0, 10);
	m_flNextSpeech = RandomFloat(0, 10);

	m_iHoverParticles = CParticleSystemLibrary::AddInstance(L"tank-hover", GetOrigin());
	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::GetInstance(m_iHoverParticles)->FollowEntity(this);
}

void CMenuMarcher::Think()
{
	BaseClass::Think();

	Speak();

	Vector vecNewOrigin = GetOrigin() + Vector(0, 0, 5) * GameServer()->GetFrameTime();
	vecNewOrigin.y = FindHoverHeight(vecNewOrigin);
	if (vecNewOrigin.z > 80)
		vecNewOrigin.z -= 160;
	SetOrigin(vecNewOrigin);
}

Vector CMenuMarcher::GetRenderOrigin() const
{
	float flLerp = 0;
	
	float flScale = fabs(fmod(GameServer()->GetGameTime()+m_flBobOffset, 4)-2)/2;
	flLerp = SLerp(flScale, 0.2f);

	return GetOrigin() + Vector(0, 1 + flLerp*0.5f, 0);
}

void CMenuMarcher::ModifyContext(CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	pContext->SetColorSwap(Color(0, 0, 255));
}

void CMenuMarcher::OnRender()
{
	BaseClass::OnRender();

	RenderTurret();
}

void CMenuMarcher::RenderTurret()
{
	if (m_iTurretModel == ~0)
		return;

	CRenderingContext r(GameServer()->GetRenderer());
	r.Translate(Vector(-0.527677f, 0.810368f, 0));

	float flScale = 1.3f;
	r.Scale(flScale, flScale, flScale);

	r.RenderModel(m_iTurretModel);
}

void CMenuMarcher::Speak()
{
	if (GameServer()->GetGameTime() < m_flNextSpeech)
		return;

	m_flNextSpeech = GameServer()->GetGameTime() + RandomFloat(5, 10);

	if (rand()%6 != 0)
		return;

	const char* apszTankLines[] =
	{
		"^.^",
		"<3",
		":D!",
		"\\o/",
		">D",
		"8)",
		"!!",
	};

	DigitanksGame()->TankSpeak(this, apszTankLines[RandomInt(0, 6)]);
}

float CMenuMarcher::FindHoverHeight(Vector vecPosition) const
{
	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	float flHighestTerrain = pTerrain->GetHeight(vecPosition.x, vecPosition.z);

	float flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	return flHighestTerrain;
}
