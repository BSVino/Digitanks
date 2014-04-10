#include "physics.h"

class CPhysicsManager
{
public:
	CPhysicsManager();
	~CPhysicsManager();

public:
	static CPhysicsModel*   GetModel(size_t i);

private:
	tvector<CPhysicsModel*> m_apModel;

	static CPhysicsManager  s_apPhysicsManager;
};

CPhysicsManager::CPhysicsManager()
{
}

CPhysicsManager::~CPhysicsManager()
{
	for (size_t i = 0; i < m_apModel.size(); i++)
		delete m_apModel[i];
}

extern CPhysicsModel* CreatePhysicsModel();

CPhysicsModel* CPhysicsManager::GetModel(size_t i)
{
	while (s_apPhysicsManager.m_apModel.size() <= i)
		s_apPhysicsManager.m_apModel.push_back(nullptr);

	if (!s_apPhysicsManager.m_apModel[i])
		s_apPhysicsManager.m_apModel[i] = CreatePhysicsModel();

	return s_apPhysicsManager.m_apModel[i];
}

CPhysicsManager CPhysicsManager::s_apPhysicsManager;

CPhysicsModel* GamePhysics()
{
	return CPhysicsManager::GetModel(PHYSWORLD_GAME);
}

CPhysicsModel* EditorPhysics()
{
	return CPhysicsManager::GetModel(PHYSWORLD_EDITOR);
}

CPhysicsModel* Physics(physics_world_t ePhysWorld)
{
	return CPhysicsManager::GetModel(ePhysWorld);
}
