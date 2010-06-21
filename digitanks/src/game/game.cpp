#include "game.h"

#include <ui/digitankswindow.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <renderer/dissolver.h>
#include <sound/sound.h>
#include <network/network.h>

#include "camera.h"

CGame* CGame::s_pGame = NULL;

CGame::CGame()
{
	assert(!s_pGame);
	s_pGame = this;

	m_bLoading = true;

	m_flRealTime = 0;
	m_flGameTime = 0;
	m_flFrameTime = 0;

	for (size_t i = 0; i < CBaseEntity::s_aEntityRegistration.size(); i++)
		CBaseEntity::s_aEntityRegistration[i].m_pfnRegisterCallback();

	m_pCamera = new CCamera();
	m_pCamera->SnapDistance(120);
	m_pRenderer = new CRenderer(CDigitanksWindow::Get()->GetWindowWidth(), CDigitanksWindow::Get()->GetWindowHeight());

	m_iClient = -1;

#ifdef _DEBUG
	CParticleSystemLibrary::Get()->LoadParticleSystem(0);
#endif
}

CGame::~CGame()
{
	delete m_pRenderer;
	delete m_pCamera;
	assert(s_pGame == this);
	s_pGame = NULL;
}

void CGame::RegisterNetworkFunctions()
{
	CNetwork::RegisterFunction("ClientInfo", this, ClientInfoCallback, 2, NET_INT, NET_FLOAT);
	CNetwork::RegisterFunction("CreateEntity", this, CreateEntityCallback, 3, NET_INT, NET_HANDLE, NET_INT);
	CNetwork::RegisterFunction("DestroyEntity", this, DestroyEntityCallback, 1, NET_INT);
	CNetwork::RegisterFunction("LoadingDone", this, LoadingDoneCallback, 0);
	CNetwork::RegisterFunction("SetOrigin", this, SetOriginCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
	CNetwork::RegisterFunction("SetAngles", this, SetAnglesCallback, 4, NET_HANDLE, NET_FLOAT, NET_FLOAT, NET_FLOAT);
}

void CGame::ClientConnect(CNetworkParameters* p)
{
	CNetwork::CallFunction(p->i2, "ClientInfo", p->i2, GetGameTime());

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		CNetwork::CallFunction(p->i2, "CreateEntity", CBaseEntity::FindRegisteredEntity(pEntity->GetClassName()), pEntity->GetHandle(), pEntity->GetSpawnSeed());
	}

	OnClientConnect(p);

	// Update entities after all creations have been run, so we don't refer to entities that haven't been created yet.
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		pEntity->ClientUpdate(p->i2);
	}

	OnClientUpdate(p);

	CNetwork::CallFunction(p->i2, "LoadingDone");
}

void CGame::LoadingDone(CNetworkParameters* p)
{
	m_bLoading = false;
}

void CGame::ClientDisconnect(CNetworkParameters* p)
{
	OnClientDisconnect(p);
}

void CGame::Think(float flRealTime)
{
	m_flFrameTime = flRealTime - m_flRealTime;

	if (m_flFrameTime > 0.15f)
		m_flFrameTime = 0.15f;

	m_flGameTime += m_flFrameTime;
	m_flRealTime = flRealTime;

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	CNetwork::Think();

	Simulate();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		pEntity->Think();
	}

	Think();

	CParticleSystemLibrary::Simulate();
	CModelDissolver::Simulate();
}

void CGame::Simulate()
{
	// Move all entities
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		pEntity->SetLastOrigin(pEntity->GetOrigin());
		pEntity->SetOrigin(pEntity->GetOrigin() + pEntity->GetVelocity() * m_flFrameTime);
		pEntity->SetVelocity(pEntity->GetVelocity() + pEntity->GetGravity() * m_flFrameTime);
	}

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (pEntity->IsDeleted())
			continue;

		for (size_t j = 0; j < CBaseEntity::GetNumEntities(); j++)
		{
			CBaseEntity* pEntity2 = CBaseEntity::GetEntityNumber(j);

			if (pEntity2->IsDeleted())
				continue;

			if (!pEntity->ShouldTouch(pEntity2))
				continue;

			Vector vecPoint;
			if (pEntity->IsTouching(pEntity2, vecPoint))
			{
				pEntity->SetOrigin(vecPoint);
				pEntity->Touching(pEntity2);
			}
		}
	}
}

void CGame::Render()
{
	m_pCamera->Think();

	m_pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	m_pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());

	m_pRenderer->SetupFrame();
	m_pRenderer->DrawBackground();
	m_pRenderer->StartRendering();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Render();

	CParticleSystemLibrary::Render();
	CModelDissolver::Render();

	m_pRenderer->FinishRendering();
}

CEntityHandle<CBaseEntity> CGame::Create(const char* pszEntityName)
{
	if (!CNetwork::ShouldRunClientFunction())
		return CEntityHandle<CBaseEntity>();

	size_t iRegisteredEntity = CBaseEntity::FindRegisteredEntity(pszEntityName);

	if (iRegisteredEntity == ~0)
		return CEntityHandle<CBaseEntity>();

	CEntityHandle<CBaseEntity> hEntity(CreateEntity(iRegisteredEntity));

	CNetwork::CallFunction(-1, "CreateEntity", iRegisteredEntity, hEntity->GetHandle(), hEntity->GetSpawnSeed());

	return hEntity;
}

size_t CGame::CreateEntity(size_t iRegisteredEntity, size_t iHandle, size_t iSpawnSeed)
{
	CBaseEntity::s_iOverrideEntityListIndex = iHandle;
	iHandle = CBaseEntity::s_aEntityRegistration[iRegisteredEntity].m_pfnCreateCallback();
	CBaseEntity::s_iOverrideEntityListIndex = ~0;

	CEntityHandle<CBaseEntity> hEntity(iHandle);

	if (iSpawnSeed)
		hEntity->SetSpawnSeed(iSpawnSeed);
	else
		hEntity->SetSpawnSeed(rand()%99999);	// Don't pick a number so large that it can't fit in (int)

	hEntity->Spawn();
	return iHandle;
}

void CGame::Delete(CBaseEntity* pEntity)
{
	if (!CNetwork::ShouldRunClientFunction())
		return;

	CNetwork::CallFunction(-1, "DestroyEntity", pEntity->GetHandle());

	CNetworkParameters p;
	p.i1 = (int)pEntity->GetHandle();
	DestroyEntity(&p);
}

void CGame::CreateEntity(CNetworkParameters* p)
{
	if (CBaseEntity::s_aEntityRegistration.size() <= (size_t)p->i1)
		return;

	CreateEntity(p->i1, p->ui2, p->i3);
}

void CGame::DestroyEntity(CNetworkParameters* p)
{
	CBaseEntity* pEntity = CBaseEntity::GetEntity(p->i1);

	CSoundLibrary::EntityDeleted(pEntity);

	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	pEntity->OnDeleted();
	OnDeleted(pEntity);
	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}

void CGame::ClientInfo(CNetworkParameters* p)
{
	m_iClient = p->i1;
	m_flGameTime = p->fl2;
}

void CGame::SetOrigin(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetOrigin(Vector(p->fl2, p->fl3, p->fl4));
}

void CGame::SetAngles(CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hEntity(p->ui1);

	if (hEntity != NULL)
		hEntity->SetAngles(EAngle(p->fl2, p->fl3, p->fl4));
}
