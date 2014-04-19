#ifndef TINKER_PLAYER_H
#define TINKER_PLAYER_H

#include <tengine/game/entities/baseentity.h>
#include <keys.h>

#include <ui/instructor.h>

class CCharacter;

class CPlayer : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CPlayer, CBaseEntity);

	class CLessonProgress
	{
	public:
		CLessonProgress()
		{
			m_flLastTimeShowed = 0;
			m_iTimesLearned = 0;
			m_flLastTimeLearned = 0;
		}

	public:
		tstring        m_sLessonName;
		double         m_flLastTimeShowed;
		int            m_iTimesLearned;
		double         m_flLastTimeLearned;
	};

public:
									CPlayer();

public:
	virtual void					MouseMotion(int dx, int dy);
	virtual bool					MouseInput(int iButton, tinker_mouse_state_t iState) { return false; };
	virtual void					KeyPress(int c);
	virtual void					KeyRelease(int c);
	virtual void					JoystickButtonPress(int iJoystick, int c);
	virtual void					JoystickButtonRelease(int iJoystick, int c) {};
	virtual void					JoystickAxis(int iJoystick, int iAxis, float flValue, float flChange);
	virtual void TouchMotion(int iFinger, float x, float y, float dx, float dy) { if (iFinger == 0) MouseMotion((int)dx, (int)dy); };
	virtual bool TouchInput(int iFinger, tinker_mouse_state_t iState, float x, float y) { if (iFinger == 0) return MouseInput(TINKER_KEY_MOUSE_LEFT, iState); else return false; }

	virtual void                    Think();

	virtual void                    Instructor_Initialize();
	virtual void                    Instructor_Think();
	virtual void                    Instructor_Respawn();

	virtual void                    Instructor_LessonLearned(const tstring& sLesson);
	virtual bool                    Instructor_IsLessonLearned(const CLessonProgress* pLessonProgress);
	virtual bool                    Instructor_IsLessonValid(const CLessonProgress* pLessonProgress);
	virtual class CLessonProgress*  Instructor_GetBestLesson();

	void							SetCharacter(CCharacter* pCharacter);
	CCharacter*						GetCharacter() const;

	void							SetClient(int iClient);
	int								GetClient() const { return m_iClient; };
	void							SetInstallID(int i) { m_iInstallID = i; };
	int								GetInstallID() const { return m_iInstallID; };

	bool							IsHumanPlayable() { return m_bHumanPlayable; }
	void							SetNotHumanPlayable() { m_bHumanPlayable = false; }

	bool							IsHumanControlled() const { return m_iClient != NETWORK_BOT; };

	void							SetPlayerName(const tstring& sName) { m_sPlayerName = sName; }
	const tstring&					GetPlayerName() const { return m_sPlayerName; }

	void							SetColor(Color clrTeam) { m_clrTeam = clrTeam; };
	Color							GetColor() const { return m_clrTeam; };

protected:
	CNetworkedColor					m_clrTeam;

	CNetworkedVariable<bool>		m_bHumanPlayable;

	CNetworkedHandle<CCharacter>	m_hCharacter;

	CNetworkedVariable<int>			m_iClient;
	size_t							m_iInstallID;

	CNetworkedString				m_sPlayerName;

	tmap<tstring, CLessonProgress>  m_apLessonProgress;
	tvector<CLessonProgress*>       m_apLessonPriorities;
	double                          m_flLastLesson;

	Vector2D                        m_vecJoystickViewVelocity;
	float                           m_flLastAcceleration;
};

#endif
