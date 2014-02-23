#ifndef DT_INTRO_SCRIPT_H
#define DT_INTRO_SCRIPT_H

#include <tmap.h>
#include <tstring.h>
#include <tvector.h>

#include <vector.h>

#include <game/entities/baseentity.h>

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

	tstring								m_sName;
	Vector								m_vecOrigin;
	EAngle								m_angAngles;
	size_t								m_iParticleInstance;
	tstring								m_sOutput;
	tstring								m_sArgs;
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
	tmap<tstring, tvector<CScriptActor> > m_aActors;
	tvector<CScriptEvent>				m_aEvents;
	double								m_flStartTime;
};

class CScriptManager
{
public:
										CScriptManager();

public:
	void								PlayScript(tstring sName);

	void								Think();

	void								ClearScripts();
	CScript*							AddScript(tstring sName);

protected:
	CScript*							m_pCurrentScript;
	tmap<tstring, CScript>				m_aScripts;
};

inline CScriptManager* ScriptManager()
{
	static CScriptManager oMgr;

	return &oMgr;
}

#endif
