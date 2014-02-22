#include <renderer/particles.h>

void CParticleSystemLibrary::InitSystems()
{
	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	CParticleSystem* pIntroExplosion = pPSL->GetParticleSystem(pPSL->AddParticleSystem("intro-explosion"));
	CParticleSystem* pIntroExplosionClouds = pPSL->GetParticleSystem(pPSL->AddParticleSystem("intro-explosion-clouds"));
	CParticleSystem* pIntroExplosionHaze = pPSL->GetParticleSystem(pPSL->AddParticleSystem("intro-explosion-haze"));

	pIntroExplosion->AddChild(pIntroExplosionClouds);
	pIntroExplosion->AddChild(pIntroExplosionHaze);

	pIntroExplosionClouds->SetMaterialName("textures/particles/cloud-white.mat");
	pIntroExplosionClouds->SetLifeTime(0.5f);
	pIntroExplosionClouds->SetEmissionRate(0.01f);
	pIntroExplosionClouds->SetEmissionMax(150);
	pIntroExplosionClouds->SetEmissionMaxDistance(300);
	pIntroExplosionClouds->SetAlpha(0.9f);
	pIntroExplosionClouds->SetStartRadius(140.0f);
	pIntroExplosionClouds->SetEndRadius(280.0f);
	pIntroExplosionClouds->SetFadeOut(1.0f);
	pIntroExplosionClouds->SetInheritedVelocity(0.0f);
	pIntroExplosionClouds->SetRandomVelocity(AABB(Vector(-10, -10, -100), Vector(10, 10, 100)));
	pIntroExplosionClouds->SetDrag(0.95f);
	pIntroExplosionClouds->SetRandomBillboardYaw(true);

	pIntroExplosionHaze->SetMaterialName("textures/particles/haze-white.mat");
	pIntroExplosionHaze->SetLifeTime(5.0f);
	pIntroExplosionHaze->SetEmissionRate(0.1f);
	pIntroExplosionHaze->SetEmissionMax(10);
	pIntroExplosionHaze->SetEmissionMaxDistance(600);
	pIntroExplosionHaze->SetAlpha(0.5f);
	pIntroExplosionHaze->SetStartRadius(440.0f);
	pIntroExplosionHaze->SetEndRadius(680.0f);
	pIntroExplosionHaze->SetFadeOut(1.0f);
	pIntroExplosionHaze->SetInheritedVelocity(0.0f);
	pIntroExplosionHaze->SetRandomVelocity(AABB(Vector(-10, -10, -100), Vector(10, 10, 100)));
	pIntroExplosionHaze->SetDrag(0.95f);
	pIntroExplosionHaze->SetRandomBillboardYaw(true);

	CParticleSystem* pIntroExplosionFragments = pPSL->GetParticleSystem(pPSL->AddParticleSystem("intro-explosion-fragments"));

	pIntroExplosionFragments->SetModel("models/intro/screen-fragment.toy");
	pIntroExplosionFragments->SetBlend(BLEND_NONE);
	pIntroExplosionFragments->SetLifeTime(5.0f);
	pIntroExplosionFragments->SetEmissionRate(0.01f);
	pIntroExplosionFragments->SetEmissionMax(20);
	pIntroExplosionFragments->SetEmissionMaxDistance(50);
	pIntroExplosionFragments->SetAlpha(1.0f);
	pIntroExplosionFragments->SetStartRadius(400.0f);
	pIntroExplosionFragments->SetEndRadius(400.0f);
	pIntroExplosionFragments->SetFadeOut(1.0f);
	pIntroExplosionFragments->SetInheritedVelocity(0.0f);
	pIntroExplosionFragments->SetRandomVelocity(AABB(Vector(-400, -200, -200), Vector(400, 200, 500)));
	pIntroExplosionFragments->SetGravity(Vector(0, -500, 0));
	pIntroExplosionFragments->SetDrag(0.95f);
	pIntroExplosionFragments->SetRandomModelYaw(true);
	pIntroExplosionFragments->SetRandomModelRoll(true);

	size_t iTankFire = pPSL->AddParticleSystem("tank-fire");
	size_t iTankFireFlash = pPSL->AddParticleSystem("tank-fire-flash");
	size_t iTankFireSmoke = pPSL->AddParticleSystem("tank-fire-smoke");

	CParticleSystem* pTankFire = pPSL->GetParticleSystem(iTankFire);
	CParticleSystem* pTankFireFlash = pPSL->GetParticleSystem(iTankFireFlash);
	CParticleSystem* pTankFireSmoke = pPSL->GetParticleSystem(iTankFireSmoke);

	pTankFire->AddChild(iTankFireFlash);
	pTankFire->AddChild(iTankFireSmoke);

	pTankFireFlash->SetMaterialName("textures/particles/tank-fire.mat");
	pTankFireFlash->SetLifeTime(0.1f);
	pTankFireFlash->SetEmissionMax(1);
	pTankFireFlash->SetAlpha(0.5f);
	pTankFireFlash->SetRadius(40.0f);
	pTankFireFlash->SetFadeOut(1.0f);
	pTankFireFlash->SetRandomBillboardYaw(true);

	pTankFireSmoke->SetMaterialName("textures/particles/haze-white.mat");
	pTankFireSmoke->SetLifeTime(0.5f);
	pTankFireSmoke->SetEmissionMax(4);
	pTankFireSmoke->SetEmissionRate(0.01f);
	pTankFireSmoke->SetAlpha(0.3f);
	pTankFireSmoke->SetStartRadius(30.0f);
	pTankFireSmoke->SetEndRadius(60.0f);
	pTankFireSmoke->SetFadeOut(1.0f);
	pTankFireSmoke->SetInheritedVelocity(0.3f);
	pTankFireSmoke->SetRandomVelocity(AABB(Vector(-6, -6, -6), Vector(6, 6, 6)));
	pTankFireSmoke->SetDrag(0.3f);
	pTankFireSmoke->SetRandomBillboardYaw(true);

	size_t iTrail = pPSL->AddParticleSystem("shell-trail");
	size_t iTrailSparks = pPSL->AddParticleSystem("shell-trail-sparks");
	size_t iTrailSparksDark = pPSL->AddParticleSystem("shell-trail-sparks-dark");
	size_t iTrailAura = pPSL->AddParticleSystem("shell-trail-aura");
	size_t iTrailWireframe1 = pPSL->AddParticleSystem("shell-trail-wf1");
	size_t iTrailWireframe2 = pPSL->AddParticleSystem("shell-trail-wf2");
	size_t iTrailWireframe3 = pPSL->AddParticleSystem("shell-trail-wf3");

	CParticleSystem* pTrail = pPSL->GetParticleSystem(iTrail);
	CParticleSystem* pTrailSparks = pPSL->GetParticleSystem(iTrailSparks);
	CParticleSystem* pTrailSparksDark = pPSL->GetParticleSystem(iTrailSparksDark);
	CParticleSystem* pTrailAura = pPSL->GetParticleSystem(iTrailAura);
	CParticleSystem* pTrailWireframe1 = pPSL->GetParticleSystem(iTrailWireframe1);
	CParticleSystem* pTrailWireframe2 = pPSL->GetParticleSystem(iTrailWireframe2);
	CParticleSystem* pTrailWireframe3 = pPSL->GetParticleSystem(iTrailWireframe3);

	pTrail->AddChild(iTrailSparks);
	pTrail->AddChild(iTrailSparksDark);
	pTrail->AddChild(iTrailAura);
	pTrail->AddChild(iTrailWireframe1);
	pTrail->AddChild(iTrailWireframe2);
	pTrail->AddChild(iTrailWireframe3);

	pTrailSparks->SetMaterialName("textures/particles/cloud-white.mat");
	pTrailSparks->SetLifeTime(1.0f);
	pTrailSparks->SetEmissionRate(0.05f);
	pTrailSparks->SetAlpha(0.7f);
	pTrailSparks->SetStartRadius(10.0f);
	pTrailSparks->SetEndRadius(5.0f);
	pTrailSparks->SetFadeOut(1.0f);
	pTrailSparks->SetInheritedVelocity(0.5f);
	pTrailSparks->SetRandomVelocity(AABB(Vector(-10, -10, -10), Vector(10, 10, 10)));
	pTrailSparks->SetDrag(0.5f);
	pTrailSparks->SetRandomBillboardYaw(true);

	pTrailSparksDark->SetMaterialName("textures/particles/cloud-white.mat");
	pTrailSparksDark->SetBlend(BLEND_ALPHA);
	pTrailSparksDark->SetColor(Color(0, 0, 0, 255));
	pTrailSparksDark->SetLifeTime(1.0f);
	pTrailSparksDark->SetEmissionRate(0.01f);
	pTrailSparksDark->SetEmissionMaxDistance(5);
	pTrailSparksDark->SetAlpha(0.9f);
	pTrailSparksDark->SetStartRadius(6);
	pTrailSparksDark->SetEndRadius(2);
	pTrailSparksDark->SetFadeOut(1.0f);
	pTrailSparksDark->SetInheritedVelocity(0.5f);
	pTrailSparksDark->SetRandomVelocity(AABB(Vector(-10, -10, -10), Vector(10, 10, 10)));
	pTrailSparksDark->SetDrag(0.5f);
	pTrailSparksDark->SetRandomBillboardYaw(true);

	pTrailAura->SetMaterialName("textures/particles/haze-white.mat");
	pTrailAura->SetLifeTime(1.0f);
	pTrailAura->SetEmissionRate(0.1f);
	pTrailAura->SetAlpha(0.05f);
	pTrailAura->SetStartRadius(40.0f);
	pTrailAura->SetEndRadius(10.0f);
	pTrailAura->SetFadeOut(0.5f);
	pTrailAura->SetInheritedVelocity(0.7f);
	pTrailAura->SetDrag(0.8f);
	pTrailAura->SetRandomBillboardYaw(true);

	pTrailWireframe1->SetMaterialName("textures/particles/wireframe1.mat");
	pTrailWireframe1->SetLifeTime(1.0f);
	pTrailWireframe1->SetEmissionRate(1.0f);
	pTrailWireframe1->SetAlpha(0.2f);
	pTrailWireframe1->SetStartRadius(0.8f);
	pTrailWireframe1->SetEndRadius(6);
	pTrailWireframe1->SetFadeOut(5);
	pTrailWireframe1->SetInheritedVelocity(0.5f);
	pTrailWireframe1->SetRandomVelocity(AABB(Vector(-60, -60, -60), Vector(60, 60, 60)));
	pTrailWireframe1->SetDrag(0.7f);
	pTrailWireframe1->SetRandomBillboardYaw(true);

	pTrailWireframe2->SetMaterialName("textures/particles/wireframe2.mat");
	pTrailWireframe2->SetLifeTime(1.0f);
	pTrailWireframe2->SetEmissionRate(1.0f);
	pTrailWireframe2->SetAlpha(0.2f);
	pTrailWireframe2->SetStartRadius(0.8f);
	pTrailWireframe2->SetEndRadius(6);
	pTrailWireframe2->SetFadeOut(5);
	pTrailWireframe2->SetInheritedVelocity(0.5f);
	pTrailWireframe2->SetRandomVelocity(AABB(Vector(-60, -60, -60), Vector(60, 60, 60)));
	pTrailWireframe2->SetDrag(0.7f);
	pTrailWireframe2->SetRandomBillboardYaw(true);

	pTrailWireframe3->SetMaterialName("textures/particles/wireframe3.mat");
	pTrailWireframe3->SetLifeTime(1.0f);
	pTrailWireframe3->SetEmissionRate(1.0f);
	pTrailWireframe3->SetAlpha(0.2f);
	pTrailWireframe3->SetStartRadius(8);
	pTrailWireframe3->SetEndRadius(6);
	pTrailWireframe3->SetFadeOut(0.5f);
	pTrailWireframe3->SetInheritedVelocity(0.5f);
	pTrailWireframe3->SetRandomVelocity(AABB(Vector(-60, -60, -60), Vector(60, 60, 60)));
	pTrailWireframe3->SetDrag(0.7f);
	pTrailWireframe3->SetRandomBillboardYaw(true);

	size_t iExplosion = pPSL->AddParticleSystem("explosion");
	size_t iExplosionBolts1 = pPSL->AddParticleSystem("explosion-bolts1");
	size_t iExplosionBolts2 = pPSL->AddParticleSystem("explosion-bolts2");

	CParticleSystem* pExplosion = pPSL->GetParticleSystem(iExplosion);
	CParticleSystem* pExplosionBolts1 = pPSL->GetParticleSystem(iExplosionBolts1);
	CParticleSystem* pExplosionBolts2 = pPSL->GetParticleSystem(iExplosionBolts2);

	pExplosion->AddChild(iExplosionBolts1);
	pExplosion->AddChild(iExplosionBolts2);

	pExplosionBolts1->SetMaterialName("textures/particles/lightning-bolt-1.mat");
	pExplosionBolts1->SetLifeTime(1.0f);
	pExplosionBolts1->SetEmissionRate(0.0f);
	pExplosionBolts1->SetEmissionMax(20);
	pExplosionBolts1->SetAlpha(0.9f);
	pExplosionBolts1->SetStartRadius(40.0f);
	pExplosionBolts1->SetEndRadius(60.0f);
	pExplosionBolts1->SetFadeOut(1.0f);
	pExplosionBolts1->SetInheritedVelocity(0.0f);
	pExplosionBolts1->SetRandomVelocity(AABB(Vector(-80, -80, -80), Vector(80, 80, 80)));
	pExplosionBolts1->SetDrag(0.95f);
	pExplosionBolts1->SetRandomBillboardYaw(true);

	pExplosionBolts2->SetMaterialName("textures/particles/lightning-bolt-2.mat");
	pExplosionBolts2->SetBlend(BLEND_ALPHA);
	pExplosionBolts2->SetColor(Color(0, 0, 0, 255));
	pExplosionBolts2->SetLifeTime(1.0f);
	pExplosionBolts2->SetEmissionRate(0.0f);
	pExplosionBolts2->SetEmissionMax(20);
	pExplosionBolts2->SetAlpha(0.4f);
	pExplosionBolts2->SetStartRadius(40.0f);
	pExplosionBolts2->SetEndRadius(60.0f);
	pExplosionBolts2->SetFadeOut(1.0f);
	pExplosionBolts2->SetInheritedVelocity(0.0f);
	pExplosionBolts2->SetRandomVelocity(AABB(Vector(-80, -80, -80), Vector(80, 80, 80)));
	pExplosionBolts2->SetDrag(0.95f);
	pExplosionBolts2->SetRandomBillboardYaw(true);
}
