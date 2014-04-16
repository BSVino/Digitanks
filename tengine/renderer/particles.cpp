#include "particles.h"

#include <maths.h>
#include <mtrand.h>
#include <tvector.h>

#include <game/entities/game.h>
#include <renderer/shaders.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <models/models.h>
#include <textures/materiallibrary.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <ui/gamewindow.h>

CParticleSystemLibrary* CParticleSystemLibrary::s_pParticleSystemLibrary = NULL;
static CParticleSystemLibrary g_pParticleSystemLibrary = CParticleSystemLibrary();

extern void InitSystems();

CParticleSystemLibrary::CParticleSystemLibrary()
{
	m_iQuadVBO = ~0;

	s_pParticleSystemLibrary = this;
	m_iParticleSystemsLoaded = 0;

	m_iSystemInstanceIndex = 0;

	InitSystems();
}

CParticleSystemLibrary::~CParticleSystemLibrary()
{
	for (size_t i = 0; i < m_apParticleSystems.size(); i++)
	{
		delete m_apParticleSystems[i];
	}

	s_pParticleSystemLibrary = NULL;
}

size_t CParticleSystemLibrary::AddParticleSystem(const tstring& sName)
{
	m_apParticleSystems.push_back(new CParticleSystem(sName));

	return m_apParticleSystems.size()-1;
}

size_t CParticleSystemLibrary::FindParticleSystem(const tstring& sName)
{
	for (size_t i = 0; i < m_apParticleSystems.size(); i++)
	{
		if (m_apParticleSystems[i]->GetName() == sName)
			return i;
	}

	return ~0;
}

void CParticleSystemLibrary::LoadParticleSystem(size_t iSystem)
{
	if (iSystem >= m_apParticleSystems.size())
		return;

	m_apParticleSystems[iSystem]->Load();
}

CParticleSystem* CParticleSystemLibrary::GetParticleSystem(size_t i)
{
	if (i >= m_apParticleSystems.size())
		return NULL;

	return m_apParticleSystems[i];
}

void CParticleSystemLibrary::MakeQuad()
{
	if (Get()->m_iQuadVBO != ~0)
		return;

	CRenderingContext c(GameServer()->GetRenderer());

	float flRadius = 1;
	Vector vecOrigin(0, 0, 0);

	Vector vecParticleUp = Vector(0, 0, flRadius);
	Vector vecParticleRight = Vector(0, flRadius, 0);

	Vector vecTL = vecOrigin - vecParticleRight + vecParticleUp;
	Vector vecTR = vecOrigin + vecParticleRight + vecParticleUp;
	Vector vecBL = vecOrigin - vecParticleRight - vecParticleUp;
	Vector vecBR = vecOrigin + vecParticleRight - vecParticleUp;

	c.BeginRenderTris();

	c.TexCoord(0.0f, 1.0f);
	c.Vertex(vecTL);
	c.TexCoord(1.0f, 0.0f);
	c.Vertex(vecBR);
	c.TexCoord(1.0f, 1.0f);
	c.Vertex(vecTR);
	c.TexCoord(0.0f, 1.0f);
	c.Vertex(vecTL);
	c.TexCoord(0.0f, 0.0f);
	c.Vertex(vecBL);
	c.TexCoord(1.0f, 0.0f);
	c.Vertex(vecBR);

	c.CreateVBO(Get()->m_iQuadVBO, Get()->m_iQuadVBOSize);
}

void CParticleSystemLibrary::Simulate()
{
	TPROF("CParticleSystemLibrary::Simulate");

	CParticleSystemLibrary* pPSL = Get();

	tmap<size_t, CSystemInstance*>::iterator it = pPSL->m_apInstances.begin();

	tvector<size_t> aiDeleted;

	for (; it != pPSL->m_apInstances.end(); it++)
	{
		CSystemInstance* pInstance = (*it).second;

		pInstance->Simulate();

		if (pInstance->IsStopped() && pInstance->GetNumParticles() == 0)
			aiDeleted.push_back((*it).first);
	}

	for (size_t i = 0; i < aiDeleted.size(); i++)
		RemoveInstance(aiDeleted[i]);
}

void CParticleSystemLibrary::Render()
{
	TPROF("CParticleSystemLibrary::Render");

	MakeQuad();

	CParticleSystemLibrary* pPSL = Get();

	if (!pPSL->m_apInstances.size())
		return;

	tmap<size_t, CSystemInstance*>::iterator it;

	if (true)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);
		c.UseProgram("particle");
		c.SetUniform("bDiffuse", true);
		c.SetUniform("iDiffuse", 0);

		for (it = pPSL->m_apInstances.begin(); it != pPSL->m_apInstances.end(); it++)
		{
			CSystemInstance* pInstance = (*it).second;
			pInstance->Render(&c, false);
		}
	}

	if (true)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);
		c.UseProgram("particle");
		c.SetUniform("bDiffuse", true);
		c.SetUniform("iDiffuse", 0);
		c.SetDepthMask(false);

		for (it = pPSL->m_apInstances.begin(); it != pPSL->m_apInstances.end(); it++)
		{
			CSystemInstance* pInstance = (*it).second;
			pInstance->Render(&c, true);
		}
	}
}

size_t CParticleSystemLibrary::AddInstance(const tstring& sName, Vector vecOrigin, EAngle angAngles)
{
	return AddInstance(CParticleSystemLibrary::Get()->FindParticleSystem(sName), vecOrigin, angAngles);
}

size_t CParticleSystemLibrary::AddInstance(size_t iParticleSystem, Vector vecOrigin, EAngle angAngles)
{
	CParticleSystemLibrary* pPSL = Get();
	CParticleSystem* pSystem = pPSL->GetParticleSystem(iParticleSystem);

	if (!pSystem)
		return ~0;

	pPSL->m_apInstances[pPSL->m_iSystemInstanceIndex++] = new CSystemInstance(pSystem, vecOrigin, angAngles);
	return pPSL->m_iSystemInstanceIndex-1;
}

void CParticleSystemLibrary::StopInstance(size_t iInstance)
{
	CSystemInstance* pInstance = GetInstance(iInstance);

	if (!pInstance)
		return;

	pInstance->Stop();
}

void CParticleSystemLibrary::StopInstances(const tstring& sName)
{
	CParticleSystemLibrary* pPSL = Get();
	tmap<size_t, CSystemInstance*>::iterator it = pPSL->m_apInstances.begin();

	for (; it != pPSL->m_apInstances.end(); it++)
	{
		CSystemInstance* pSystemInstance = (*it).second;
		if (pSystemInstance->GetSystem()->GetName() == sName)
			pSystemInstance->Stop();
	}
}

void CParticleSystemLibrary::RemoveInstance(size_t iInstance)
{
	tmap<size_t, CSystemInstance*>::iterator it = Get()->m_apInstances.find(iInstance);

	if (it == Get()->m_apInstances.end())
		return;

	delete (*it).second;
	Get()->m_apInstances.erase(it);
}

CSystemInstance* CParticleSystemLibrary::GetInstance(size_t iInstance)
{
	if (iInstance == ~0)
		return NULL;

	if (Get()->m_apInstances.find(iInstance) == Get()->m_apInstances.end())
		return NULL;

	return Get()->m_apInstances[iInstance];
}

void CParticleSystemLibrary::ClearInstances()
{
	CParticleSystemLibrary* pPSL = Get();
	while (pPSL->m_apInstances.size())
		RemoveInstance((*pPSL->m_apInstances.begin()).first);
}

void CParticleSystemLibrary::ReloadSystems()
{
	ClearInstances();

	CParticleSystemLibrary* pPSL = Get();

	for (size_t i = 0; i < pPSL->m_apParticleSystems.size(); i++)
		delete pPSL->m_apParticleSystems[i];

	pPSL->m_apParticleSystems.clear();

	InitSystems();

	for (size_t i = 0; i < pPSL->m_apParticleSystems.size(); i++)
		pPSL->LoadParticleSystem(i);
}

void CParticleSystemLibrary::ResetReferenceCounts()
{
	for (size_t i = 0; i < Get()->m_apParticleSystems.size(); i++)
	{
		if (!Get()->m_apParticleSystems[i])
			continue;

		Get()->m_apParticleSystems[i]->SetReferences(0);
	}
}

void CParticleSystemLibrary::ClearUnreferenced()
{
	while (true)
	{
		bool bUnloaded = false;

		for (size_t i = 0; i < Get()->m_apParticleSystems.size(); i++)
		{
			if (!Get()->m_apParticleSystems[i])
				continue;

			if (!Get()->m_apParticleSystems[i]->IsLoaded())
				continue;

			if (!Get()->m_apParticleSystems[i]->GetReferences())
			{
				Get()->m_apParticleSystems[i]->Unload();
				bUnloaded = true;
				break;
			}
		}

		// If we unloaded something we need to start again in case it had a dependency earlier in the list.
		if (!bUnloaded)
			break;
	}
}

void ReloadParticles(CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	CParticleSystemLibrary::ReloadSystems();
}

CCommand particles_reload("particles_reload", ReloadParticles);

CParticleSystem::CParticleSystem(tstring sName)
{
	m_iReferences = 0;
	m_bLoaded = false;
	m_sName = sName;

	m_iModel = 0;

	m_eBlend = BLEND_ADDITIVE;
	m_flLifeTime = 1.0f;
	m_flEmissionRate = 0.1f;
	m_iEmissionMax = 0;
	m_flEmissionMaxDistance = 0;
	m_flAlpha = 1.0f;
	m_clrColor = Color(255, 255, 255, 255);
	m_flStartRadius = 1.0f;
	m_flEndRadius = 1.0f;
	m_flFadeIn = 0.0f;
	m_flFadeOut = 0.25f;
	m_flInheritedVelocity = 0.0f;
	m_vecGravity = Vector(0, 0, -10);
	m_flDrag = 1.0f;
	m_bRandomBillboardYaw = false;
	m_bRandomModelYaw = true;
	m_bRandomModelRoll = true;
	m_bRandomAngleVelocity = false;
}

CParticleSystem::~CParticleSystem()
{
	Unload();
}

void CParticleSystem::Load()
{
	if (IsLoaded())
		return;

	m_iReferences = 1;
	m_bLoaded = true;

	if (GetMaterialName().length() > 0)
		SetMaterial(CMaterialLibrary::AddMaterial(GetMaterialName()));

	if (GetModelName().length() > 0)
		SetModel(CModelLibrary::AddModel(GetModelName()));

	CParticleSystemLibrary::Get()->m_iParticleSystemsLoaded++;

	for (size_t i = 0; i < GetNumChildren(); i++)
		CParticleSystemLibrary::Get()->GetParticleSystem(GetChild(i))->Load();
}

void CParticleSystem::Unload()
{
	if (!IsLoaded())
		return;

	TAssert(m_iReferences == 0);

	m_bLoaded = false;

	SetMaterial(CMaterialHandle());

	if (GetModelName().length() > 0)
		CModelLibrary::ReleaseModel(GetModelName());

	CParticleSystemLibrary::Get()->m_iParticleSystemsLoaded--;

	for (size_t i = 0; i < GetNumChildren(); i++)
		CParticleSystemLibrary::Get()->GetParticleSystem(GetChild(i))->Unload();
}

bool CParticleSystem::IsRenderable()
{
	return !!GetMaterial() || !!GetModel();
}

void CParticleSystem::AddChild(size_t iSystem)
{
	m_aiChildren.push_back(iSystem);
}

void CParticleSystem::AddChild(CParticleSystem* pSystem)
{
	for (size_t i = 0; i < CParticleSystemLibrary::GetNumParticleSystems(); i++)
	{
		if (CParticleSystemLibrary::Get()->GetParticleSystem(i) == pSystem)
			AddChild(i);
	}
}

CSystemInstance::CSystemInstance(CParticleSystem* pSystem, Vector vecOrigin, EAngle angAngles)
{
	m_pSystem = pSystem;
	m_vecOrigin = vecOrigin;
	m_angAngles = angAngles;

	m_bStopped = false;

	m_iNumParticlesAlive = 0;

	m_flLastEmission = GameServer()->GetGameTime() - RandomFloat(0, m_pSystem->GetEmissionRate()) - 0.01f;
	m_iTotalEmitted = 0;

	CParticleSystemLibrary* pPSL = CParticleSystemLibrary::Get();

	for (size_t i = 0; i < m_pSystem->GetNumChildren(); i++)
		m_apChildren.push_back(new CSystemInstance(pPSL->GetParticleSystem(m_pSystem->GetChild(i)), vecOrigin, m_angAngles));

	m_bColorOverride = false;
}

CSystemInstance::~CSystemInstance()
{
	for (size_t i = 0; i < m_pSystem->GetNumChildren(); i++)
		delete m_apChildren[i];
}

void CSystemInstance::Simulate()
{
	double flGameTime = GameServer()->GetGameTime();
	double flFrameTime = GameServer()->GetFrameTime();

	if (m_hFollow != NULL)
	{
		m_vecOrigin = m_hFollow->BaseGetRenderOrigin();
		m_vecInheritedVelocity = m_hFollow->GetGlobalVelocity();
	}

	for (size_t i = 0; i < m_aParticles.size(); i++)
	{
		CParticle* pParticle = &m_aParticles[i];

		if (!pParticle->m_bActive)
			continue;

		float flLifeTime = (float)(flGameTime - pParticle->m_flSpawnTime);
		if (flLifeTime > m_pSystem->GetLifeTime())
		{
			pParticle->m_bActive = false;
			m_iNumParticlesAlive--;
			continue;
		}

		pParticle->m_vecOrigin += pParticle->m_vecVelocity * (float)flFrameTime;
		pParticle->m_vecVelocity += m_pSystem->GetGravity() * (float)flFrameTime;
		pParticle->m_vecVelocity *= (1-((1-m_pSystem->GetDrag()) * (float)flFrameTime));

		if (m_pSystem->GetRandomAngleVelocity())
			pParticle->m_angAngles = (pParticle->m_angAngles + pParticle->m_angAngleVelocity*(float)GameServer()->GetFrameTime());

		float flLifeTimeRamp = flLifeTime / m_pSystem->GetLifeTime();

		float flFadeIn = 1;
		float flFadeOut = 1;

		if (flLifeTimeRamp < m_pSystem->GetFadeIn())
			flFadeIn = RemapVal(flLifeTimeRamp, 0, m_pSystem->GetFadeIn(), 0, 1);

		if (flLifeTimeRamp > 1-m_pSystem->GetFadeOut())
			flFadeOut = RemapVal(flLifeTimeRamp, 1-m_pSystem->GetFadeOut(), 1, 1, 0);

		pParticle->m_flAlpha = flFadeIn * flFadeOut * m_pSystem->GetAlpha();

		pParticle->m_flRadius = RemapVal(flLifeTimeRamp, 0, 1, m_pSystem->GetStartRadius(), m_pSystem->GetEndRadius());
	}

	if (!m_bStopped && m_pSystem->IsRenderable())
	{
		while (flGameTime - m_flLastEmission > m_pSystem->GetEmissionRate())
		{
			SpawnParticle();

			if (m_pSystem->GetEmissionMax() && m_iTotalEmitted >= m_pSystem->GetEmissionMax())
				break;

			m_flLastEmission += m_pSystem->GetEmissionRate();
		}
	}

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Simulate();

	if (m_pSystem->GetEmissionMax() && m_iTotalEmitted >= m_pSystem->GetEmissionMax() || !m_pSystem->IsRenderable())
		m_bStopped = true;

	if (!m_bStopped && m_aParticles.size())
	{
		auto& oParticle = m_aParticles[0];
		float& flRadius = oParticle.m_flRadius;
		Vector vecRadius = Vector(flRadius, flRadius, flRadius);
		m_aabbBounds.m_vecMins = oParticle.m_vecOrigin - vecRadius;
		m_aabbBounds.m_vecMaxs = oParticle.m_vecOrigin + vecRadius;

		for (size_t i = 1; i < m_aParticles.size(); i++)
		{
			auto& oParticle = m_aParticles[i];

			if (!oParticle.m_bActive)
				continue;

			float& flRadius = oParticle.m_flRadius;
			Vector vecRadius = Vector(flRadius, flRadius, flRadius);

			m_aabbBounds.Expand(AABB(oParticle.m_vecOrigin - vecRadius, oParticle.m_vecOrigin + vecRadius));
		}
	}
}

void CSystemInstance::SpawnParticle()
{
	m_iNumParticlesAlive++;
	m_iTotalEmitted++;

	CParticle* pNewParticle = NULL;

	for (size_t i = 0; i < m_aParticles.size(); i++)
	{
		CParticle* pParticle = &m_aParticles[i];

		if (pParticle->m_bActive)
			continue;

		pNewParticle = pParticle;
		break;
	}

	if (!pNewParticle)
	{
		m_aParticles.push_back(CParticle());
		pNewParticle = &m_aParticles[m_aParticles.size()-1];
	}

	Vector vecDistance = Vector(0,0,0);
	if (m_pSystem->GetEmissionMaxDistance() > 0)
	{
		float flYaw = RandomFloat(-180, 180);
		float flDistance = cos(RandomFloat(0, M_PI/2)) * m_pSystem->GetEmissionMaxDistance();
		float flPitch = sin(RandomFloat(-M_PI/2, M_PI/2)) * 90;
		vecDistance = AngleVector(EAngle(flPitch, flYaw, 0)) * flDistance;
	}

	pNewParticle->Reset();
	pNewParticle->m_vecOrigin = m_vecOrigin + m_pSystem->GetSpawnOffset() + vecDistance;
	pNewParticle->m_vecVelocity = m_vecInheritedVelocity * m_pSystem->GetInheritedVelocity();

	if (m_pSystem->GetRandomVelocity().Size().LengthSqr() > 0)
	{
		Vector vecMins = m_pSystem->GetRandomVelocity().m_vecMins;
		Vector vecMaxs = m_pSystem->GetRandomVelocity().m_vecMaxs;
		pNewParticle->m_vecVelocity.x += RandomFloat(vecMins.x, vecMaxs.x);
		pNewParticle->m_vecVelocity.y += RandomFloat(vecMins.y, vecMaxs.y);
		pNewParticle->m_vecVelocity.z += RandomFloat(vecMins.z, vecMaxs.z);
	}

	pNewParticle->m_angAngles = m_angAngles;

	if (m_pSystem->GetRandomModelYaw())
		pNewParticle->m_angAngles.y = RandomFloat(0, 360);

	if (m_pSystem->GetRandomModelRoll())
		pNewParticle->m_angAngles.r = RandomFloat(-180, 180);

	if (m_pSystem->GetRandomAngleVelocity())
		pNewParticle->m_angAngleVelocity = EAngle(RandomFloat(-90, 90), RandomFloat(-180, 180), RandomFloat(-90, 90));

	if (m_pSystem->GetFadeIn())
		pNewParticle->m_flAlpha = 0;
	else
		pNewParticle->m_flAlpha = m_pSystem->GetAlpha();

	pNewParticle->m_flRadius = m_pSystem->GetStartRadius();

	if (m_pSystem->GetRandomBillboardYaw())
		pNewParticle->m_flBillboardYaw = RandomFloat(0, 360);
	else
		pNewParticle->m_flBillboardYaw = 0;
}

CVar particles_debug("particles_debug", "0");

void CSystemInstance::Render(CGameRenderingContext* c, bool bTransparent)
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Render(c, bTransparent);

	if (m_pSystem->GetBlend() == BLEND_NONE && bTransparent)
		return;

	if (m_pSystem->GetBlend() != BLEND_NONE && !bTransparent)
		return;

	CGameRenderer* pRenderer = GameWindow()->GetGameRenderer();

	if (particles_debug.GetBool())
	{
		CRenderingContext c(pRenderer, true);
		c.UseProgram("debug");
		c.SetUniform("vecColor", Color(1.0, 1.0, 1.0, 1.0f));
		c.ResetTransformations();
		c.RenderWireBox(m_aabbBounds);
	}

	if (!pRenderer->IsSphereInFrustum(m_aabbBounds.Center(), m_aabbBounds.Size().Length() / 2))
		return;

	Vector vecForward, vecLeft, vecUp;
	pRenderer->GetCameraVectors(&vecForward, &vecLeft, &vecUp);

	if (m_pSystem->GetMaterial())
		c->UseMaterial(m_pSystem->GetMaterial());

	c->SetBlend(m_pSystem->GetBlend());

	Color clrParticle = m_pSystem->GetColor();
	if (m_bColorOverride)
		clrParticle = m_clrOverride;

	if (m_pSystem->GetModel())
	{
		int iRadius = c->GetUniform("flRadius");

		for (size_t i = 0; i < m_aParticles.size(); i++)
		{
			CParticle* pParticle = &m_aParticles[i];

			if (!pParticle->m_bActive)
				continue;

			c->SetUniform(iRadius, pParticle->m_flAlpha);
			c->SetColor(clrParticle);
			c->Translate(pParticle->m_vecOrigin);
			c->Rotate(-pParticle->m_angAngles.y, Vector(0, 0, 1));
			c->Rotate(pParticle->m_angAngles.p, Vector(0, 1, 0));
			c->Rotate(pParticle->m_angAngles.r, Vector(1, 0, 0));
			c->Scale(pParticle->m_flRadius, pParticle->m_flRadius, pParticle->m_flRadius);
			c->RenderModel(m_pSystem->GetModel());
			c->ResetTransformations();
		}
	}
	else
	{
		c->SetUniform("vecCameraPosition", GameServer()->GetRenderer()->GetCameraPosition());

		size_t iQuadVBO = CParticleSystemLibrary::Get()->GetQuadVBO();
		size_t iQuadVBOSize = CParticleSystemLibrary::Get()->GetQuadVBOSize();

		int iOrigin = c->GetUniform("vecOrigin");
		int iAlpha = c->GetUniform("flAlpha");
		int iRadius = c->GetUniform("flRadius");
		int iYaw = c->GetUniform("flYaw");
		int iColor = c->GetUniform("vecColor");

		for (size_t i = 0; i < m_aParticles.size(); i++)
		{
			CParticle* pParticle = &m_aParticles[i];

			if (!pParticle->m_bActive)
				continue;

			c->SetUniform(iOrigin, pParticle->m_vecOrigin);
			c->SetUniform(iAlpha, pParticle->m_flAlpha);
			c->SetUniform(iRadius, pParticle->m_flRadius);
			c->SetUniform(iYaw, pParticle->m_flBillboardYaw*M_PI/180);
			c->SetUniform(iColor, clrParticle);

			c->BeginRenderVertexArray(iQuadVBO);
			c->SetPositionBuffer(0u, 20);
			c->SetTexCoordBuffer(12, 20);
			c->EndRenderVertexArray(iQuadVBOSize);
		}
	}
}

void CSystemInstance::FollowEntity(CBaseEntity* pFollow)
{
	m_hFollow = pFollow;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->FollowEntity(pFollow);
}

void CSystemInstance::SetInheritedVelocity(Vector vecInheritedVelocity)
{
	m_vecInheritedVelocity = vecInheritedVelocity;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->SetInheritedVelocity(vecInheritedVelocity);
}

void CSystemInstance::Stop()
{
	m_bStopped = true;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->Stop();
}

bool CSystemInstance::IsStopped()
{
	for (size_t i = 0; i < m_apChildren.size(); i++)
	{
		if (!m_apChildren[i]->IsStopped())
			return false;
	}

	return m_bStopped;
}

size_t CSystemInstance::GetNumParticles()
{
	size_t iAlive = 0;
	for (size_t i = 0; i < m_apChildren.size(); i++)
		iAlive += m_apChildren[i]->GetNumParticles();

	return iAlive + m_iNumParticlesAlive;
}

void CSystemInstance::SetColor(Color c)
{
	m_bColorOverride = true;
	m_clrOverride = c;

	for (size_t i = 0; i < m_apChildren.size(); i++)
		m_apChildren[i]->SetColor(c);
}

CParticle::CParticle()
{
	Reset();
}

void CParticle::Reset()
{
	m_vecOrigin = m_vecVelocity = Vector();
	m_angAngles = EAngle();
	m_flAlpha = 1;
	m_flRadius = 1;
	m_bActive = true;
	m_flSpawnTime = GameServer()->GetGameTime();
}

CParticleSystemInstanceHandle::CParticleSystemInstanceHandle()
{
	m_iSystem = ~0;
	m_hFollow = NULL;
	m_vecOrigin = Vector();

	m_iInstance = ~0;
}

CParticleSystemInstanceHandle::~CParticleSystemInstanceHandle()
{
	if (m_iInstance != ~0)
		CParticleSystemLibrary::StopInstance(m_iInstance);
}

void CParticleSystemInstanceHandle::SetSystem(const tstring& sSystem, Vector vecOrigin)
{
	SetSystem(CParticleSystemLibrary::Get()->FindParticleSystem(sSystem), vecOrigin);
}

void CParticleSystemInstanceHandle::SetSystem(size_t iSystem, Vector vecOrigin)
{
	m_iSystem = iSystem;
	m_hFollow = NULL;
	m_vecOrigin = vecOrigin;

	m_iInstance = ~0;
	SetActive(false);
}

void CParticleSystemInstanceHandle::SetActive(bool bActive)
{
	if (bActive && m_iInstance == ~0)
	{
		// Light up the night
		m_iInstance = CParticleSystemLibrary::AddInstance(m_iSystem, m_vecOrigin);
		if (m_iInstance != ~0 && m_hFollow != NULL)
			CParticleSystemLibrary::GetInstance(m_iInstance)->FollowEntity(m_hFollow);
	}
	else if (!bActive && m_iInstance != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iInstance);
		m_iInstance = ~0;
	}
}
