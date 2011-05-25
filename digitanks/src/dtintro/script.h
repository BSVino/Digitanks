#ifndef DT_INTRO_SCRIPT_H
#define DT_INTRO_SCRIPT_H

#include <EASTL/map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <vector.h>

#include <game/baseentity.h>

class CScriptActor
{
public:
	CEntityHandle<CBaseEntity>			m_hEntity;

	Vector								m_vecOldOrigin;
	EAngle								m_angOldAngles;
	Vector								m_vecNewOrigin;
	EAngle								m_angNewAngles;
};

typedef enum
{
	EC_UNDEFINED = 0,
	EC_MOVEACTOR,
	EC_PARTICLES,
	EC_FIREOUTPUT,
} eventclass_t;

class CScriptEvent
{
public:
										CScriptEvent();

public:
	eventclass_t						m_eEventClass;
	float								m_flStartTime;
	float								m_flEndTime;
	bool								m_bStarted;
	bool								m_bExecuted;

	eastl::string						m_sName;
	Vector								m_vecOrigin;
	EAngle								m_angAngles;
	size_t								m_iParticleInstance;
	eastl::string						m_sOutput;
	eastl::string16						m_sArgs;
};

class CScript
{
public:
	void								Play();
	void								Think();
	void								PlayEvent(size_t i);
	void								ThinkEvent(size_t i);
	void								FinishEvent(size_t i);

	CScriptEvent*						AddScriptEvent();

protected:
	eastl::map<eastl::string, eastl::vector<CScriptActor> > m_aActors;
	eastl::vector<CScriptEvent>			m_aEvents;
	float								m_flStartTime;
};

class CScriptManager
{
public:
										CScriptManager();

public:
	void								PlayScript(eastl::string sName);

	void								Think();

	void								ClearScripts();
	CScript*							AddScript(eastl::string sName);

protected:
	CScript*							m_pCurrentScript;
	eastl::map<eastl::string, CScript>	m_aScripts;
};

inline CScriptManager* ScriptManager()
{
	static CScriptManager oMgr;

	return &oMgr;
}

#endif
