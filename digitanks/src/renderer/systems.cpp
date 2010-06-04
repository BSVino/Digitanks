#include "particles.h"

void CParticleSystemLibrary::InitSystems()
{
	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	size_t iTrail = pPSL->AddParticleSystem(L"shell-trail");
	size_t iTrailSparks = pPSL->AddParticleSystem(L"shell-trail-sparks");
	size_t iTrailAura = pPSL->AddParticleSystem(L"shell-trail-aura");

	CParticleSystem* pTrail = pPSL->GetParticleSystem(iTrail);
	CParticleSystem* pTrailSparks = pPSL->GetParticleSystem(iTrailSparks);
	CParticleSystem* pTrailAura = pPSL->GetParticleSystem(iTrailAura);

	pTrail->AddChild(iTrailSparks);
	pTrail->AddChild(iTrailAura);

	pTrailSparks->SetTexture(L"textures/particles/cloud-white.png");
	pTrailSparks->SetLifeTime(1.0f);
	pTrailSparks->SetEmissionRate(0.05f);
	pTrailSparks->SetAlpha(0.7f);
	pTrailSparks->SetStartRadius(1.0f);
	pTrailSparks->SetEndRadius(0.5f);
	pTrailSparks->SetFadeOut(1.0f);
	pTrailSparks->SetInheritedVelocity(0.5f);
	pTrailSparks->SetRandomVelocity(AABB(Vector(-1, -1, -1), Vector(1, 1, 1)));
	pTrailSparks->SetDrag(0.5f);
	pTrailSparks->SetRandomBillboardYaw(true);

	pTrailAura->SetTexture(L"textures/particles/haze-white.png");
	pTrailAura->SetLifeTime(1.0f);
	pTrailAura->SetEmissionRate(0.1f);
	pTrailAura->SetAlpha(0.05f);
	pTrailAura->SetStartRadius(4.0f);
	pTrailAura->SetEndRadius(1.0f);
	pTrailAura->SetFadeOut(0.5f);
	pTrailAura->SetInheritedVelocity(0.7f);
	pTrailAura->SetDrag(0.8f);
	pTrailAura->SetRandomBillboardYaw(true);

	size_t iTankFire = pPSL->AddParticleSystem(L"tank-fire");
	size_t iTankFireFlash = pPSL->AddParticleSystem(L"tank-fire-flash");
	size_t iTankFireSmoke = pPSL->AddParticleSystem(L"tank-fire-smoke");

	CParticleSystem* pTankFire = pPSL->GetParticleSystem(iTankFire);
	CParticleSystem* pTankFireFlash = pPSL->GetParticleSystem(iTankFireFlash);
	CParticleSystem* pTankFireSmoke = pPSL->GetParticleSystem(iTankFireSmoke);

	pTankFire->AddChild(iTankFireFlash);
	pTankFire->AddChild(iTankFireSmoke);

	pTankFire->SetTexture(L"textures/particles/tank-fire.png");
	pTankFire->SetLifeTime(0.1f);
	pTankFire->SetEmissionMax(1);
	pTankFire->SetAlpha(0.5f);
	pTankFire->SetRadius(4.0f);
	pTankFire->SetFadeOut(1.0f);
	pTankFire->SetRandomBillboardYaw(true);

	pTankFireSmoke->SetTexture(L"textures/particles/haze-white.png");
	pTankFireSmoke->SetLifeTime(0.5f);
	pTankFireSmoke->SetEmissionMax(4);
	pTankFireSmoke->SetEmissionRate(0.01f);
	pTankFireSmoke->SetAlpha(0.3f);
	pTankFireSmoke->SetStartRadius(3.0f);
	pTankFireSmoke->SetEndRadius(6.0f);
	pTankFireSmoke->SetFadeOut(1.0f);
	pTankFireSmoke->SetInheritedVelocity(0.3f);
	pTankFireSmoke->SetRandomVelocity(AABB(Vector(-6, -6, -6), Vector(6, 6, 6)));
	pTankFireSmoke->SetDrag(0.3f);
	pTankFireSmoke->SetRandomBillboardYaw(true);
}
