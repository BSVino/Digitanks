#include <renderer/particles.h>

void CParticleSystemLibrary::InitSystems()
{
	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	CParticleSystem* pIntroExplosion = pPSL->GetParticleSystem(pPSL->AddParticleSystem(L"intro-explosion"));
	CParticleSystem* pIntroExplosionClouds = pPSL->GetParticleSystem(pPSL->AddParticleSystem(L"intro-explosion-clouds"));
	CParticleSystem* pIntroExplosionHaze = pPSL->GetParticleSystem(pPSL->AddParticleSystem(L"intro-explosion-haze"));

	pIntroExplosion->AddChild(pIntroExplosionClouds);
	pIntroExplosion->AddChild(pIntroExplosionHaze);

	pIntroExplosionClouds->SetTexture(L"textures/particles/cloud-white.png");
	pIntroExplosionClouds->SetLifeTime(0.5f);
	pIntroExplosionClouds->SetEmissionRate(0.01f);
	pIntroExplosionClouds->SetEmissionMax(150);
	pIntroExplosionClouds->SetEmissionMaxDistance(300);
	pIntroExplosionClouds->SetAlpha(0.9f);
	pIntroExplosionClouds->SetStartRadius(140.0f);
	pIntroExplosionClouds->SetEndRadius(280.0f);
	pIntroExplosionClouds->SetFadeOut(1.0f);
	pIntroExplosionClouds->SetInheritedVelocity(0.0f);
	pIntroExplosionClouds->SetRandomVelocity(AABB(Vector(-10, -10, -10), Vector(10, 10, 10)));
	pIntroExplosionClouds->SetDrag(0.95f);
	pIntroExplosionClouds->SetRandomBillboardYaw(true);

	pIntroExplosionHaze->SetTexture(L"textures/particles/haze-white.png");
	pIntroExplosionHaze->SetLifeTime(5.0f);
	pIntroExplosionHaze->SetEmissionRate(0.1f);
	pIntroExplosionHaze->SetEmissionMax(10);
	pIntroExplosionHaze->SetEmissionMaxDistance(600);
	pIntroExplosionHaze->SetAlpha(0.9f);
	pIntroExplosionHaze->SetStartRadius(440.0f);
	pIntroExplosionHaze->SetEndRadius(680.0f);
	pIntroExplosionHaze->SetFadeOut(1.0f);
	pIntroExplosionHaze->SetInheritedVelocity(0.0f);
	pIntroExplosionHaze->SetRandomVelocity(AABB(Vector(-10, -10, -10), Vector(10, 10, 10)));
	pIntroExplosionHaze->SetDrag(0.95f);
	pIntroExplosionHaze->SetRandomBillboardYaw(true);
}
