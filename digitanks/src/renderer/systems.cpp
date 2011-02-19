#include "particles.h"

void CParticleSystemLibrary::InitSystems()
{
	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	size_t iTrail = pPSL->AddParticleSystem(L"shell-trail");
	size_t iTrailSparks = pPSL->AddParticleSystem(L"shell-trail-sparks");
	size_t iTrailAura = pPSL->AddParticleSystem(L"shell-trail-aura");
	size_t iTrailWireframe1 = pPSL->AddParticleSystem(L"shell-trail-wf1");
	size_t iTrailWireframe2 = pPSL->AddParticleSystem(L"shell-trail-wf2");
	size_t iTrailWireframe3 = pPSL->AddParticleSystem(L"shell-trail-wf3");

	CParticleSystem* pTrail = pPSL->GetParticleSystem(iTrail);
	CParticleSystem* pTrailSparks = pPSL->GetParticleSystem(iTrailSparks);
	CParticleSystem* pTrailAura = pPSL->GetParticleSystem(iTrailAura);
	CParticleSystem* pTrailWireframe1 = pPSL->GetParticleSystem(iTrailWireframe1);
	CParticleSystem* pTrailWireframe2 = pPSL->GetParticleSystem(iTrailWireframe2);
	CParticleSystem* pTrailWireframe3 = pPSL->GetParticleSystem(iTrailWireframe3);

	pTrail->AddChild(iTrailSparks);
	pTrail->AddChild(iTrailAura);
	pTrail->AddChild(iTrailWireframe1);
	pTrail->AddChild(iTrailWireframe2);
	pTrail->AddChild(iTrailWireframe3);

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

	pTrailWireframe1->SetTexture(L"textures/particles/wireframe1.png");
	pTrailWireframe1->SetLifeTime(1.0f);
	pTrailWireframe1->SetEmissionRate(1.0f);
	pTrailWireframe1->SetAlpha(0.2f);
	pTrailWireframe1->SetStartRadius(0.8f);
	pTrailWireframe1->SetEndRadius(0.6f);
	pTrailWireframe1->SetFadeOut(0.5f);
	pTrailWireframe1->SetInheritedVelocity(0.5f);
	pTrailWireframe1->SetRandomVelocity(AABB(Vector(-6, -6, -6), Vector(6, 6, 6)));
	pTrailWireframe1->SetDrag(0.7f);
	pTrailWireframe1->SetRandomBillboardYaw(true);

	pTrailWireframe2->SetTexture(L"textures/particles/wireframe2.png");
	pTrailWireframe2->SetLifeTime(1.0f);
	pTrailWireframe2->SetEmissionRate(1.0f);
	pTrailWireframe2->SetAlpha(0.2f);
	pTrailWireframe2->SetStartRadius(0.8f);
	pTrailWireframe2->SetEndRadius(0.6f);
	pTrailWireframe2->SetFadeOut(0.5f);
	pTrailWireframe2->SetInheritedVelocity(0.5f);
	pTrailWireframe2->SetRandomVelocity(AABB(Vector(-6, -6, -6), Vector(6, 6, 6)));
	pTrailWireframe2->SetDrag(0.7f);
	pTrailWireframe2->SetRandomBillboardYaw(true);

	pTrailWireframe3->SetTexture(L"textures/particles/wireframe3.png");
	pTrailWireframe3->SetLifeTime(1.0f);
	pTrailWireframe3->SetEmissionRate(1.0f);
	pTrailWireframe3->SetAlpha(0.2f);
	pTrailWireframe3->SetStartRadius(0.8f);
	pTrailWireframe3->SetEndRadius(0.6f);
	pTrailWireframe3->SetFadeOut(0.5f);
	pTrailWireframe3->SetInheritedVelocity(0.5f);
	pTrailWireframe3->SetRandomVelocity(AABB(Vector(-6, -6, -6), Vector(6, 6, 6)));
	pTrailWireframe3->SetDrag(0.7f);
	pTrailWireframe3->SetRandomBillboardYaw(true);

	size_t iTankFire = pPSL->AddParticleSystem(L"tank-fire");
	size_t iTankFireFlash = pPSL->AddParticleSystem(L"tank-fire-flash");
	size_t iTankFireSmoke = pPSL->AddParticleSystem(L"tank-fire-smoke");

	CParticleSystem* pTankFire = pPSL->GetParticleSystem(iTankFire);
	CParticleSystem* pTankFireFlash = pPSL->GetParticleSystem(iTankFireFlash);
	CParticleSystem* pTankFireSmoke = pPSL->GetParticleSystem(iTankFireSmoke);

	pTankFire->AddChild(iTankFireFlash);
	pTankFire->AddChild(iTankFireSmoke);

	pTankFireFlash->SetTexture(L"textures/particles/tank-fire.png");
	pTankFireFlash->SetLifeTime(0.1f);
	pTankFireFlash->SetEmissionMax(1);
	pTankFireFlash->SetAlpha(0.5f);
	pTankFireFlash->SetRadius(4.0f);
	pTankFireFlash->SetFadeOut(1.0f);
	pTankFireFlash->SetRandomBillboardYaw(true);

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

	size_t iPromotion = pPSL->AddParticleSystem(L"promotion");
	size_t iPromotionStar = pPSL->AddParticleSystem(L"promotion-star");
	size_t iPromotionGlow = pPSL->AddParticleSystem(L"promotion-glow");

	CParticleSystem* pPromotion = pPSL->GetParticleSystem(iPromotion);
	CParticleSystem* pPromotionStar = pPSL->GetParticleSystem(iPromotionStar);
	CParticleSystem* pPromotionGlow = pPSL->GetParticleSystem(iPromotionGlow);

	pPromotion->AddChild(iPromotionStar);
	pPromotion->AddChild(iPromotionGlow);

	pPromotionStar->SetTexture(L"textures/particles/promotion.png");
	pPromotionStar->SetLifeTime(2.0f);
	pPromotionStar->SetEmissionMax(1);
	pPromotionStar->SetAlpha(0.5f);
	pPromotionStar->SetRadius(3.0f);
	pPromotionStar->SetFadeOut(0.5f);
	pPromotionStar->SetSpawnOffset(Vector(0, 6, 0));
	pPromotionStar->SetRandomVelocity(AABB(Vector(0, 0.5f, 0), Vector(0, 0.5f, 0)));
	pPromotionStar->SetGravity(Vector(0, 3, 0));

	pPromotionGlow->SetTexture(L"textures/particles/haze-white.png");
	pPromotionGlow->SetLifeTime(1.5f);
	pPromotionGlow->SetEmissionMax(3);
	pPromotionGlow->SetEmissionRate(0.01f);
	pPromotionGlow->SetAlpha(0.3f);
	pPromotionGlow->SetColor(Color(250, 200, 0));
	pPromotionGlow->SetRadius(8.0f);
	pPromotionGlow->SetFadeOut(0.5f);
	pPromotionGlow->SetSpawnOffset(Vector(0, 3, 0));
	pPromotionGlow->SetRandomVelocity(AABB(Vector(-0.1f, -0.1f, -0.1f), Vector(0.1f, 0.1f, 0.1f)));
	pPromotionGlow->SetGravity(Vector(0, 0, 0));
	pPromotionGlow->SetRandomBillboardYaw(true);

	size_t iTankHover = pPSL->AddParticleSystem(L"tank-hover");
	CParticleSystem* pTankHover = pPSL->GetParticleSystem(iTankHover);

	pTankHover->SetTexture(L"textures/particles/haze-white.png");
	pTankHover->SetLifeTime(0.5f);
	pTankHover->SetEmissionRate(0.1f);
	pTankHover->SetAlpha(0.2f);
	pTankHover->SetRadius(3.5f);
	pTankHover->SetFadeOut(0.5f);
	pTankHover->SetSpawnOffset(Vector(0, -1, 0));
	pTankHover->SetRandomVelocity(AABB(Vector(0, -1, 0), Vector(0, -2, 0)));
	pTankHover->SetGravity(Vector(0, 10, 0));

	size_t iElectronodeSpark = pPSL->AddParticleSystem(L"electronode-spark");
	CParticleSystem* pElectronodeSpark = pPSL->GetParticleSystem(iElectronodeSpark);

	pElectronodeSpark->SetTexture(L"textures/particles/electrospark.png");
	pElectronodeSpark->SetLifeTime(2.0f);
	pElectronodeSpark->SetEmissionRate(0.4f);
	pElectronodeSpark->SetAlpha(0.5f);
	pElectronodeSpark->SetRadius(1.0f);
	pElectronodeSpark->SetFadeOut(0.5f);
	pElectronodeSpark->SetSpawnOffset(Vector(0, 6, 0));
	pElectronodeSpark->SetRandomVelocity(AABB(Vector(-1, 0.5f, -1), Vector(1, 1.5f, 1)));
	pElectronodeSpark->SetGravity(Vector(0, 3, 0));

	size_t iTorpedoTrail = pPSL->AddParticleSystem(L"torpedo-trail");
	size_t iTorpedoTrailSparks = pPSL->AddParticleSystem(L"torpedo-trail-sparks");
	size_t iTorpedoTrailAura = pPSL->AddParticleSystem(L"torpedo-trail-aura");

	CParticleSystem* pTorpedoTrail = pPSL->GetParticleSystem(iTorpedoTrail);
	CParticleSystem* pTorpedoTrailSparks = pPSL->GetParticleSystem(iTorpedoTrailSparks);
	CParticleSystem* pTorpedoTrailAura = pPSL->GetParticleSystem(iTorpedoTrailAura);

	pTorpedoTrail->AddChild(iTorpedoTrailSparks);
	pTorpedoTrail->AddChild(iTorpedoTrailAura);

	pTorpedoTrailSparks->SetTexture(L"textures/particles/cloud-white.png");
	pTorpedoTrailSparks->SetLifeTime(1.0f);
	pTorpedoTrailSparks->SetEmissionRate(0.05f);
	pTorpedoTrailSparks->SetAlpha(0.9f);
	pTorpedoTrailSparks->SetStartRadius(0.5f);
	pTorpedoTrailSparks->SetEndRadius(0.2f);
	pTorpedoTrailSparks->SetFadeOut(1.0f);
	pTorpedoTrailSparks->SetInheritedVelocity(0.0f);
	pTorpedoTrailSparks->SetRandomVelocity(AABB(Vector(-5, 30, -5), Vector(5, 15, 5)));
	pTorpedoTrailSparks->SetDrag(0.95f);
	pTorpedoTrailSparks->SetRandomBillboardYaw(true);

	pTorpedoTrailAura->SetTexture(L"textures/particles/haze-white.png");
	pTorpedoTrailAura->SetLifeTime(1.0f);
	pTorpedoTrailAura->SetEmissionRate(0.1f);
	pTorpedoTrailAura->SetAlpha(0.3f);
	pTorpedoTrailAura->SetStartRadius(4.0f);
	pTorpedoTrailAura->SetEndRadius(3.0f);
	pTorpedoTrailAura->SetFadeOut(0.5f);
	pTorpedoTrailAura->SetInheritedVelocity(0.7f);
	pTorpedoTrailAura->SetDrag(0.6f);
	pTorpedoTrailAura->SetRandomBillboardYaw(true);

	size_t iTorpedoExplosion = pPSL->AddParticleSystem(L"torpedo-explosion");
	size_t iTorpedoExplosionBolts1 = pPSL->AddParticleSystem(L"torpedo-explosion-bolts1");
	size_t iTorpedoExplosionBolts2 = pPSL->AddParticleSystem(L"torpedo-explosion-bolts2");

	CParticleSystem* pTorpedoExplosion = pPSL->GetParticleSystem(iTorpedoExplosion);
	CParticleSystem* pTorpedoExplosionBolts1 = pPSL->GetParticleSystem(iTorpedoExplosionBolts1);
	CParticleSystem* pTorpedoExplosionBolts2 = pPSL->GetParticleSystem(iTorpedoExplosionBolts2);

	pTorpedoExplosion->AddChild(iTorpedoExplosionBolts1);
	pTorpedoExplosion->AddChild(iTorpedoExplosionBolts2);

	pTorpedoExplosionBolts1->SetTexture(L"textures/particles/lightning-bolt-1.png");
	pTorpedoExplosionBolts1->SetLifeTime(1.0f);
	pTorpedoExplosionBolts1->SetEmissionRate(0.0f);
	pTorpedoExplosionBolts1->SetEmissionMax(20);
	pTorpedoExplosionBolts1->SetAlpha(0.9f);
	pTorpedoExplosionBolts1->SetStartRadius(4.0f);
	pTorpedoExplosionBolts1->SetEndRadius(6.0f);
	pTorpedoExplosionBolts1->SetFadeOut(1.0f);
	pTorpedoExplosionBolts1->SetInheritedVelocity(0.0f);
	pTorpedoExplosionBolts1->SetRandomVelocity(AABB(Vector(-8, -8, -8), Vector(8, 8, 8)));
	pTorpedoExplosionBolts1->SetDrag(0.95f);
	pTorpedoExplosionBolts1->SetRandomBillboardYaw(true);

	pTorpedoExplosionBolts2->SetTexture(L"textures/particles/lightning-bolt-2.png");
	pTorpedoExplosionBolts2->SetLifeTime(1.0f);
	pTorpedoExplosionBolts2->SetEmissionRate(0.0f);
	pTorpedoExplosionBolts2->SetEmissionMax(20);
	pTorpedoExplosionBolts2->SetAlpha(0.9f);
	pTorpedoExplosionBolts2->SetStartRadius(4.0f);
	pTorpedoExplosionBolts2->SetEndRadius(6.0f);
	pTorpedoExplosionBolts2->SetFadeOut(1.0f);
	pTorpedoExplosionBolts2->SetInheritedVelocity(0.0f);
	pTorpedoExplosionBolts2->SetRandomVelocity(AABB(Vector(-8, -8, -8), Vector(8, 8, 8)));
	pTorpedoExplosionBolts2->SetDrag(0.95f);
	pTorpedoExplosionBolts2->SetRandomBillboardYaw(true);

	size_t iAoEExplosionStrategy = pPSL->AddParticleSystem(L"aoe-explosion-strategy");

	CParticleSystem* pAoEExplosionStrategy = pPSL->GetParticleSystem(iAoEExplosionStrategy);

	pAoEExplosionStrategy->SetTexture(L"textures/particles/aoe-bubble.png");
	pAoEExplosionStrategy->SetLifeTime(0.5f);
	pAoEExplosionStrategy->SetEmissionRate(0.01f);
	pAoEExplosionStrategy->SetEmissionMax(150);
	pAoEExplosionStrategy->SetEmissionMaxDistance(10);
	pAoEExplosionStrategy->SetAlpha(0.9f);
	pAoEExplosionStrategy->SetStartRadius(4.0f);
	pAoEExplosionStrategy->SetEndRadius(8.0f);
	pAoEExplosionStrategy->SetFadeOut(1.0f);
	pAoEExplosionStrategy->SetInheritedVelocity(0.0f);
	pAoEExplosionStrategy->SetRandomVelocity(AABB(Vector(-1, -1, -1), Vector(1, 1, 1)));
	pAoEExplosionStrategy->SetDrag(0.95f);
	pAoEExplosionStrategy->SetRandomBillboardYaw(true);

	size_t iAoEExplosionArtillery = pPSL->AddParticleSystem(L"aoe-explosion-artillery");

	CParticleSystem* pAoEExplosionArtillery = pPSL->GetParticleSystem(iAoEExplosionArtillery);

	pAoEExplosionArtillery->SetTexture(L"textures/particles/aoe-bubble.png");
	pAoEExplosionArtillery->SetLifeTime(0.5f);
	pAoEExplosionArtillery->SetEmissionRate(0.01f);
	pAoEExplosionArtillery->SetEmissionMax(150);
	pAoEExplosionArtillery->SetEmissionMaxDistance(25);
	pAoEExplosionArtillery->SetAlpha(0.9f);
	pAoEExplosionArtillery->SetStartRadius(6.0f);
	pAoEExplosionArtillery->SetEndRadius(12.0f);
	pAoEExplosionArtillery->SetFadeOut(1.0f);
	pAoEExplosionArtillery->SetInheritedVelocity(0.0f);
	pAoEExplosionArtillery->SetRandomVelocity(AABB(Vector(-1, -1, -1), Vector(1, 1, 1)));
	pAoEExplosionArtillery->SetDrag(0.95f);
	pAoEExplosionArtillery->SetRandomBillboardYaw(true);

	size_t iAoETrail = pPSL->AddParticleSystem(L"aoe-trail");
	size_t iAoETrailSparks = pPSL->AddParticleSystem(L"aoe-trail-sparks");

	CParticleSystem* pAoETrail = pPSL->GetParticleSystem(iAoETrail);
	CParticleSystem* pAoETrailSparks = pPSL->GetParticleSystem(iAoETrailSparks);

	pAoETrail->AddChild(iAoETrailSparks);
	pAoETrail->AddChild(iTrailAura);
	pAoETrail->AddChild(iTrailWireframe1);
	pAoETrail->AddChild(iTrailWireframe2);
	pAoETrail->AddChild(iTrailWireframe3);

	pAoETrailSparks->SetTexture(L"textures/particles/aoe-bubble.png");
	pAoETrailSparks->SetLifeTime(1.0f);
	pAoETrailSparks->SetEmissionRate(0.05f);
	pAoETrailSparks->SetAlpha(0.7f);
	pAoETrailSparks->SetStartRadius(1.3f);
	pAoETrailSparks->SetEndRadius(0.5f);
	pAoETrailSparks->SetFadeOut(1.0f);
	pAoETrailSparks->SetInheritedVelocity(0.5f);
	pAoETrailSparks->SetRandomVelocity(AABB(Vector(-5, -5, -5), Vector(5, 5, 5)));
	pAoETrailSparks->SetDrag(0.5f);
	pAoETrailSparks->SetRandomBillboardYaw(true);

	size_t iEMPExplosion = pPSL->AddParticleSystem(L"emp-explosion");
	size_t iEMPExplosionBolts1 = pPSL->AddParticleSystem(L"emp-explosion-bolts1");
	size_t iEMPExplosionBolts2 = pPSL->AddParticleSystem(L"emp-explosion-bolts2");

	CParticleSystem* pEMPExplosion = pPSL->GetParticleSystem(iEMPExplosion);
	CParticleSystem* pEMPExplosionBolts1 = pPSL->GetParticleSystem(iEMPExplosionBolts1);
	CParticleSystem* pEMPExplosionBolts2 = pPSL->GetParticleSystem(iEMPExplosionBolts2);

	pEMPExplosion->AddChild(iEMPExplosionBolts1);
	pEMPExplosion->AddChild(iEMPExplosionBolts2);

	pEMPExplosionBolts1->SetTexture(L"textures/particles/lightning-bolt-1.png");
	pEMPExplosionBolts1->SetLifeTime(1.0f);
	pEMPExplosionBolts1->SetEmissionRate(0.0f);
	pEMPExplosionBolts1->SetEmissionMax(20);
	pEMPExplosionBolts1->SetAlpha(0.9f);
	pEMPExplosionBolts1->SetStartRadius(4.0f);
	pEMPExplosionBolts1->SetEndRadius(6.0f);
	pEMPExplosionBolts1->SetFadeOut(1.0f);
	pEMPExplosionBolts1->SetInheritedVelocity(0.0f);
	pEMPExplosionBolts1->SetRandomVelocity(AABB(Vector(-12, -12, -12), Vector(12, 12, 12)));
	pEMPExplosionBolts1->SetDrag(0.95f);
	pEMPExplosionBolts1->SetRandomBillboardYaw(true);

	pEMPExplosionBolts2->SetTexture(L"textures/particles/lightning-bolt-2.png");
	pEMPExplosionBolts2->SetLifeTime(1.0f);
	pEMPExplosionBolts2->SetEmissionRate(0.0f);
	pEMPExplosionBolts2->SetEmissionMax(20);
	pEMPExplosionBolts2->SetAlpha(0.9f);
	pEMPExplosionBolts2->SetStartRadius(4.0f);
	pEMPExplosionBolts2->SetEndRadius(6.0f);
	pEMPExplosionBolts2->SetFadeOut(1.0f);
	pEMPExplosionBolts2->SetInheritedVelocity(0.0f);
	pEMPExplosionBolts2->SetRandomVelocity(AABB(Vector(-12, -12, -12), Vector(12, 12, 12)));
	pEMPExplosionBolts2->SetDrag(0.95f);
	pEMPExplosionBolts2->SetRandomBillboardYaw(true);

	size_t iEMPTrail = pPSL->AddParticleSystem(L"emp-trail");
	size_t iEMPTrailSparks1 = pPSL->AddParticleSystem(L"emp-trail-sparks1");
	size_t iEMPTrailSparks2 = pPSL->AddParticleSystem(L"emp-trail-sparks2");

	CParticleSystem* pEMPTrail = pPSL->GetParticleSystem(iEMPTrail);
	CParticleSystem* pEMPTrailSparks1 = pPSL->GetParticleSystem(iEMPTrailSparks1);
	CParticleSystem* pEMPTrailSparks2 = pPSL->GetParticleSystem(iEMPTrailSparks2);

	pEMPTrail->AddChild(iEMPTrailSparks1);
	pEMPTrail->AddChild(iEMPTrailSparks2);
	pEMPTrail->AddChild(iTrailAura);
	pEMPTrail->AddChild(iTrailWireframe1);
	pEMPTrail->AddChild(iTrailWireframe2);
	pEMPTrail->AddChild(iTrailWireframe3);

	pEMPTrailSparks1->SetTexture(L"textures/particles/lightning-bolt-1.png");
	pEMPTrailSparks1->SetLifeTime(1.0f);
	pEMPTrailSparks1->SetEmissionRate(0.05f);
	pEMPTrailSparks1->SetAlpha(0.7f);
	pEMPTrailSparks1->SetStartRadius(1.3f);
	pEMPTrailSparks1->SetEndRadius(0.5f);
	pEMPTrailSparks1->SetFadeOut(1.0f);
	pEMPTrailSparks1->SetInheritedVelocity(0.5f);
	pEMPTrailSparks1->SetRandomVelocity(AABB(Vector(-5, -5, -5), Vector(5, 5, 5)));
	pEMPTrailSparks1->SetDrag(0.5f);
	pEMPTrailSparks1->SetRandomBillboardYaw(true);

	pEMPTrailSparks2->SetTexture(L"textures/particles/lightning-bolt-2.png");
	pEMPTrailSparks2->SetLifeTime(1.0f);
	pEMPTrailSparks2->SetEmissionRate(0.05f);
	pEMPTrailSparks2->SetAlpha(0.7f);
	pEMPTrailSparks2->SetStartRadius(1.3f);
	pEMPTrailSparks2->SetEndRadius(0.5f);
	pEMPTrailSparks2->SetFadeOut(1.0f);
	pEMPTrailSparks2->SetInheritedVelocity(0.5f);
	pEMPTrailSparks2->SetRandomVelocity(AABB(Vector(-5, -5, -5), Vector(5, 5, 5)));
	pEMPTrailSparks2->SetDrag(0.5f);
	pEMPTrailSparks2->SetRandomBillboardYaw(true);

	size_t iTractorBomb = pPSL->AddParticleSystem(L"tractor-bomb-explosion");
	size_t iTractorBombPulses = pPSL->AddParticleSystem(L"tractor-bomb-explosion-pulses");
	size_t iTractorBombSmoke = pPSL->AddParticleSystem(L"tractor-bomb-explosion-smoke");

	CParticleSystem* pTractorBomb = pPSL->GetParticleSystem(iTractorBomb);
	CParticleSystem* pTractorBombPulses = pPSL->GetParticleSystem(iTractorBombPulses);
	CParticleSystem* pTractorBombSmoke = pPSL->GetParticleSystem(iTractorBombSmoke);

	pTractorBomb->AddChild(iTractorBombPulses);
	pTractorBomb->AddChild(iTractorBombSmoke);

	pTractorBombPulses->SetModel(L"models/particles/pulse.obj");
	pTractorBombPulses->SetLifeTime(0.3f);
	pTractorBombPulses->SetEmissionRate(0.15f);
	pTractorBombPulses->SetEmissionMax(3);
	pTractorBombPulses->SetAlpha(0.7f);
	pTractorBombPulses->SetStartRadius(1.0f);
	pTractorBombPulses->SetEndRadius(60.0f);
	pTractorBombPulses->SetFadeOut(1.0f);
	pTractorBombPulses->SetInheritedVelocity(0.0f);
	pTractorBombPulses->SetDrag(0.0f);

	pTractorBombSmoke->SetTexture(L"textures/particles/haze-white.png");
	pTractorBombSmoke->SetLifeTime(1.0f);
	pTractorBombSmoke->SetEmissionRate(0.01f);
	pTractorBombSmoke->SetEmissionMax(5);
	pTractorBombSmoke->SetAlpha(0.2f);
	pTractorBombSmoke->SetStartRadius(4.0f);
	pTractorBombSmoke->SetEndRadius(30.0f);
	pTractorBombSmoke->SetFadeOut(0.5f);
	pTractorBombSmoke->SetDrag(0.15f);
	pTractorBombSmoke->SetRandomVelocity(AABB(Vector(-30, -10, -30), Vector(30, 30, 30)));
	pTractorBombSmoke->SetRandomBillboardYaw(true);

	size_t iTractorBombTrail = pPSL->AddParticleSystem(L"tractor-bomb-trail");

	CParticleSystem* pTractorBombTrail = pPSL->GetParticleSystem(iTractorBombTrail);

	pTractorBombTrail->AddChild(iTrailAura);
	pTractorBombTrail->AddChild(iTrailWireframe1);
	pTractorBombTrail->AddChild(iTrailWireframe2);
	pTractorBombTrail->AddChild(iTrailWireframe3);

	size_t iBoltTrail = pPSL->AddParticleSystem(L"bolt-trail");

	CParticleSystem* pBoltTrail = pPSL->GetParticleSystem(iBoltTrail);

	pBoltTrail->AddChild(iTrailAura);
	pBoltTrail->AddChild(iTrailWireframe1);
	pBoltTrail->AddChild(iTrailWireframe2);
	pBoltTrail->AddChild(iTrailWireframe3);

	size_t iBoltExplosion = pPSL->AddParticleSystem(L"bolt-explosion");

	CParticleSystem* pBoltExplosion = pPSL->GetParticleSystem(iBoltExplosion);

	pBoltExplosion->SetTexture(L"textures/particles/cloud-white.png");
	pBoltExplosion->SetLifeTime(1.1f);
	pBoltExplosion->SetEmissionRate(0.0f);
	pBoltExplosion->SetEmissionMax(10);
	pBoltExplosion->SetAlpha(0.5f);
	pBoltExplosion->SetStartRadius(0.3f);
	pBoltExplosion->SetEndRadius(3.0f);
	pBoltExplosion->SetFadeOut(1.0f);
	pBoltExplosion->SetInheritedVelocity(0.0f);
	pBoltExplosion->SetRandomVelocity(AABB(Vector(0, 0, 0), Vector(4, 50, 4)));
	pBoltExplosion->SetDrag(0.1f);
	pBoltExplosion->SetRandomBillboardYaw(true);
}
