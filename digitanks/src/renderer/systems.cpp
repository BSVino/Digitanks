#include "particles.h"

void CParticleSystemLibrary::InitSystems()
{
	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	size_t iTest = pPSL->AddParticleSystem(L"test");
	CParticleSystem* pTest = pPSL->GetParticleSystem(iTest);

	pTest->SetTexture(L"textures/particles/cloud-white.png");

	size_t iTrail = pPSL->AddParticleSystem(L"shell-trail");
	CParticleSystem* pTrail = pPSL->GetParticleSystem(iTrail);

	pTrail->SetTexture(L"textures/particles/cloud-white.png");
	pTrail->SetLifeTime(1.0f);
	pTrail->SetEmissionRate(0.05f);
	pTrail->SetAlpha(0.7f);
	pTrail->SetStartRadius(1.0f);
	pTrail->SetEndRadius(0.5f);
	pTrail->SetFadeOut(1.0f);
	pTrail->SetInheritedVelocity(0.5f);
	pTrail->SetRandomVelocity(AABB(Vector(-1, -1, -1), Vector(1, 1, 1)));
	pTrail->SetRandomBillboardYaw(true);
}
