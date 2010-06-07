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
}
