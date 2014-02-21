#include "replication.h"

#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>

#include <game/entities/baseentity.h>
#include <game/gameserver.h>

CNetworkedVariableData::CNetworkedVariableData()
{
	m_iOffset = 0;
	m_flUpdateInterval = 0;
}

CNetworkedVariableBase* CNetworkedVariableData::GetNetworkedVariableBase(CBaseEntity* pEntity)
{
	TAssert(m_iOffset);
	return (CNetworkedVariableBase*)(((size_t)pEntity) + m_iOffset);
}

CNetworkedVariableBase::CNetworkedVariableBase()
{
	m_bDirty = true;
	m_flLastUpdate = 0;
}

CVar net_replication_debug("net_replication_debug", "off");

void CGameServerNetwork::UpdateNetworkVariables(int iClient, bool bForceAll)
{
	if (!GameNetwork()->IsConnected())
		return;

	double flTime = GameServer()->GetGameTime();

	size_t iMaxEnts = GameServer()->GetMaxEntities();
	for (size_t i = 0; i < iMaxEnts; i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		const tchar* pszClassName = pEntity->GetClassName();

		CEntityRegistration* pRegistration = NULL;
		do
		{
			pRegistration = pEntity->GetRegisteredEntity(pszClassName);

			TAssert(pRegistration);
			if (!pRegistration)
				break;

			size_t iNetVarsSize = pRegistration->m_aNetworkVariables.size();
			for (size_t j = 0; j < iNetVarsSize; j++)
			{
				CNetworkedVariableData* pVarData = &pRegistration->m_aNetworkVariables[j];
				CNetworkedVariableBase* pVariable = pVarData->GetNetworkedVariableBase(pEntity);

				if (!bForceAll)
				{
					if (!pVariable->IsDirty())
						continue;

					if (flTime - pVariable->m_flLastUpdate < pVarData->m_flUpdateInterval)
						continue;
				}

				// For one, m_flLastUpdate needs to be a double
				pVariable->m_flLastUpdate = (float)flTime;
				// For two, it's shit.
				TUnimplemented();
				// Try some testing or something.

				CNetworkParameters p;
				p.ui1 = pEntity->GetHandle();

				size_t iDataSize;
				void* pValue = pVariable->Serialize(iDataSize);

				if (net_replication_debug.GetBool())
				{
					if (iDataSize >= 4)
						TMsg(tstring("Updating ") + pVarData->GetName() + sprintf(tstring(" (%x) (%f) (%d)\n"), *(unsigned int*)pValue, *(float*)pValue, *(int*)pValue));
					else
						TMsg(tstring("Updating ") + pVarData->GetName() + "\n");
				}

				p.CreateExtraData(iDataSize + strlen(pVarData->GetName())+1);
				strcpy((char*)p.m_pExtraData, pVarData->GetName());
				memcpy((unsigned char*)(p.m_pExtraData) + strlen(pVarData->GetName())+1, pValue, iDataSize);

				// UV stands for UpdateValue
				GameNetwork()->CallFunctionParameters(iClient, "UV", &p);

				// Only reset the dirty flag if all clients got the message.
				if (iClient == NETWORK_TOCLIENTS)
					pVariable->SetDirty(false);
			}

			pszClassName = pRegistration->m_pszParentClass;
		} while (pszClassName);
	}
}
