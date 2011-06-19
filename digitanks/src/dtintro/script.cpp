#include "script.h"

#include <game/gameserver.h>
#include <renderer/particles.h>

CScriptManager::CScriptManager()
{
	m_pCurrentScript = NULL;
}

void CScriptManager::PlayScript(eastl::string sName)
{
	if (m_aScripts.find(sName) == m_aScripts.end())
		return;

	m_pCurrentScript = &m_aScripts[sName];
	m_pCurrentScript->Play();
}

void CScriptManager::Think()
{
	if (m_pCurrentScript)
		m_pCurrentScript->Think();
}

void CScriptManager::ClearScripts()
{
	m_pCurrentScript = NULL;
	m_aScripts.clear();
}

CScript* CScriptManager::AddScript(eastl::string sName)
{
	m_aScripts[sName] = CScript();
	return &m_aScripts[sName];
}

void CScript::Play()
{
	m_flStartTime = GameServer()->GetGameTime();

	m_aActors.clear();
	for (size_t i = 0; i < m_aEvents.size(); i++ )
	{
		CScriptEvent* pEvent = &m_aEvents[i];

		eastl::vector<CBaseEntity*> apEntities;
		CBaseEntity::FindEntitiesByName(pEvent->m_sName, apEntities);

		for (size_t j = 0; j < apEntities.size(); j++)
		{
			CScriptActor* pActor = &m_aActors[pEvent->m_sName].push_back();
			pActor->m_hEntity = apEntities[j];
		}
	}

	Think();
}

void CScript::Think()
{
	float flCurrentTime = GameServer()->GetGameTime() - m_flStartTime;

	for (size_t i = 0; i < m_aEvents.size(); i++ )
	{
		CScriptEvent* pEvent = &m_aEvents[i];

		if (flCurrentTime >= pEvent->m_flStartTime && !pEvent->m_bStarted)
			PlayEvent(i);
		else if (pEvent->m_flEndTime && pEvent->m_bStarted && !pEvent->m_bExecuted && flCurrentTime >= pEvent->m_flEndTime)
			FinishEvent(i);
		else if (pEvent->m_bStarted && pEvent->m_flEndTime && !pEvent->m_bExecuted)
			ThinkEvent(i);
	}
}

void CScript::PlayEvent(size_t i)
{
	CScriptEvent* pEvent = &m_aEvents[i];

	pEvent->m_bStarted = true;

	switch (pEvent->m_eEventClass)
	{
	case EC_PARTICLES:
		pEvent->m_iParticleInstance = CParticleSystemLibrary::AddInstance(convertstring<char, tchar>(pEvent->m_sName), pEvent->m_vecOrigin, pEvent->m_angAngles);
		break;

	case EC_MOVEACTOR:
	{
		for (size_t i = 0; i < m_aActors[pEvent->m_sName].size(); i++)
		{
			CScriptActor* pActor = &m_aActors[pEvent->m_sName][i];
			if (pActor->m_hEntity == NULL)
				continue;

			pActor->m_vecOldOrigin = pActor->m_hEntity->GetOrigin();
			pActor->m_angOldAngles = pActor->m_hEntity->GetAngles();

			pActor->m_vecNewOrigin = pEvent->m_vecOrigin;
			pActor->m_angNewAngles = pEvent->m_angAngles;
		}
		break;
	}

	case EC_FIREOUTPUT:
	{
		eastl::vector<CBaseEntity*> apEntities;
		CBaseEntity::FindEntitiesByName(pEvent->m_sName, apEntities);

		for (size_t j = 0; j < apEntities.size(); j++)
		{
			if (apEntities[j]->GetInput(pEvent->m_sOutput.c_str()))
				apEntities[j]->CallInput(pEvent->m_sOutput, pEvent->m_sArgs);
		}

		break;
	}
	}

	if (!pEvent->m_flEndTime)
		pEvent->m_bExecuted = true;
}

void CScript::ThinkEvent(size_t i)
{
	CScriptEvent* pEvent = &m_aEvents[i];

	switch (pEvent->m_eEventClass)
	{
	case EC_MOVEACTOR:
	{
		for (size_t i = 0; i < m_aActors[pEvent->m_sName].size(); i++)
		{
			CScriptActor* pActor = &m_aActors[pEvent->m_sName][i];
			if (pActor->m_hEntity == NULL)
				continue;

			float flLerp = SLerp(RemapVal(GameServer()->GetGameTime() - m_flStartTime, pEvent->m_flStartTime, pEvent->m_flEndTime, 0, 1), 0.2f);
			pActor->m_hEntity->SetOrigin(pActor->m_vecNewOrigin*flLerp + pActor->m_vecOldOrigin*(1-flLerp));
			pActor->m_hEntity->SetAngles(pActor->m_angNewAngles*flLerp + pActor->m_angOldAngles*(1-flLerp));
		}
		break;
	}
	}
}

void CScript::FinishEvent(size_t i)
{
	CScriptEvent* pEvent = &m_aEvents[i];

	pEvent->m_bExecuted = true;

	switch (pEvent->m_eEventClass)
	{
	case EC_PARTICLES:
	{
		CParticleSystemLibrary::StopInstance(pEvent->m_iParticleInstance);
		break;
	}

	case EC_MOVEACTOR:
	{
		for (size_t i = 0; i < m_aActors[pEvent->m_sName].size(); i++)
		{
			CScriptActor* pActor = &m_aActors[pEvent->m_sName][i];
			if (pActor->m_hEntity == NULL)
				continue;

			pActor->m_hEntity->SetOrigin(pEvent->m_vecOrigin);
			pActor->m_hEntity->SetAngles(pEvent->m_angAngles);
		}
		break;
	}
	}
}

CScriptEvent* CScript::AddScriptEvent()
{
	return &m_aEvents.push_back();
}

CScriptEvent::CScriptEvent()
{
	m_eEventClass = EC_UNDEFINED;
	m_flStartTime = 0;
	m_flEndTime = 0;
	m_bStarted = false;
	m_bExecuted = false;
}
