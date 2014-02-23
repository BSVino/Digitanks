#include "game.h"

#include <strutils.h>

#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <sound/sound.h>
#include <network/network.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <game/cameramanager.h>
#include <ui/gamewindow.h>
#include <game/level.h>

REGISTER_ENTITY(CGame);

NETVAR_TABLE_BEGIN(CGame);
	NETVAR_DEFINE_CALLBACK(CEntityHandle<CPlayer>, m_ahPlayers, &CGame::ClearLocalPlayers);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGame);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CPlayer>, m_ahPlayers);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CEntityHandle<CPlayer>, m_ahLocalPlayers);	// Detected on the fly.
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGame);
	INPUT_DEFINE(ReloadLevel);
	INPUT_DEFINE(LoadLevel);
INPUTS_TABLE_END();

CGame::CGame()
{
}

CGame::~CGame()
{
	for (size_t i = 0; i < m_ahPlayers.size(); i++)
		m_ahPlayers[i]->Delete();
}

void CGame::Spawn()
{
	BaseClass::Spawn();

	RegisterNetworkFunctions();
}

void CGame::ReloadLevel(const tvector<tstring>& asArgs)
{
	GameWindow()->QueueReloadLevel();
}

void CGame::LoadLevel(const tvector<tstring>& asArgs)
{
	tstring sLevel;
	for (size_t i = 0; i < asArgs.size(); i++)
		sLevel += asArgs[i] + " ";

	CHandle<CLevel> pLevel = GameServer()->GetLevel(sLevel);
	if (!pLevel)
		return;

	GameWindow()->Restart("level");
	CVar::SetCVar("game_level", pLevel->GetFile());
}

void CGame::RegisterNetworkFunctions()
{
}

void CGame::OnClientConnect(int iClient)
{
}

void CGame::OnClientEnterGame(int iClient)
{
}

void CGame::OnClientDisconnect(int iClient)
{
	for (size_t i = 0; i < m_ahPlayers.size(); i++)
 	{
		if (m_ahPlayers[i]->GetClient() == iClient)
 		{
			m_ahPlayers[i]->SetClient(NETWORK_BOT);
			return;
		}
	}

	TAssert(!"Couldn't find the guy who just quit!");
}

void CGame::EnterGame()
{
}

void CGame::AddPlayer(CPlayer* pPlayer)
{
	if (!pPlayer)
		return;

	// Prevent dupes
	for (size_t i = 0; i < m_ahPlayers.size(); i++)
	{
		if (pPlayer == m_ahPlayers[i])
			return;
	}

	m_ahPlayers.push_back(pPlayer);
	m_ahLocalPlayers.clear();
}

void CGame::RemovePlayer(const CPlayer* pPlayer)
{
	if (!pPlayer)
		return;

	for (size_t i = 0; i < m_ahPlayers.size(); i++)
	{
		if (pPlayer == m_ahPlayers[i])
		{
			m_ahPlayers.erase(i);
			break;
		}
	}

	m_ahLocalPlayers.clear();
}

void CGame::OnDeleted()
{
	m_ahLocalPlayers.clear();
}

void CGame::OnDeleted(const CBaseEntity* pEntity)
{
	RemovePlayer(dynamic_cast<const CPlayer*>(pEntity));
}

bool CGame::TakesDamageFrom(CBaseEntity* pVictim, CBaseEntity* pAttacker)
{
	return true;
}

CPlayer* CGame::GetPlayer(size_t i) const
{
	if (i >= m_ahPlayers.size())
		return NULL;

	return m_ahPlayers[i];
}

bool CGame::IsTeamControlledByMe(const class CPlayer* pPlayer)
{
	if (!pPlayer)
		return false;

	if (!pPlayer->IsHumanControlled())
		return false;

	for (size_t i = 0; i < GetNumLocalPlayers(); i++)
	{
		if (GetLocalPlayer(i) == pPlayer)
			return true;
	}

	return false;
}

const tvector<CEntityHandle<CPlayer> >& CGame::GetLocalPlayers()
{
	if (m_ahLocalPlayers.size() == 0)
	{
		CGameServer* pGameServer = GameServer();
		for (size_t i = 0; i < m_ahPlayers.size(); i++)
		{
			if (!m_ahPlayers[i])
				continue;

			if (m_ahPlayers[i]->GetClient() == pGameServer->GetClientIndex())
				m_ahLocalPlayers.push_back(m_ahPlayers[i]);
		}
	}

	return m_ahLocalPlayers;
}

size_t CGame::GetNumLocalPlayers()
{
	if (m_ahLocalPlayers.size() == 0)
		GetLocalPlayers();

	return m_ahLocalPlayers.size();
}

CPlayer* CGame::GetLocalPlayer(size_t i)
{
	if (m_ahLocalPlayers.size() == 0)
		GetLocalPlayers();

	return m_ahLocalPlayers[i];
}

CPlayer* CGame::GetLocalPlayer()
{
	if (m_ahLocalPlayers.size() == 0)
		GetLocalPlayers();

	TAssert(m_ahLocalPlayers.size() == 1);

	if (!m_ahLocalPlayers.size())
		return NULL;

	return m_ahLocalPlayers[0];
}

void CGame::ClearLocalPlayers(CNetworkedVariableBase* pVariable)
{
	CGame* pGame = Game();
	if (!pGame)
		return;

	pGame->m_ahLocalPlayers.clear();
}

CVar cheats("cheats", "off");

bool CGame::AllowCheats()
{
	return cheats.GetBool();
}
