#ifndef DT_INSTRUCTOR_H
#define DT_INSTRUCTOR_H

#include <map>
#include <glgui/glgui.h>

class CTutorial
{
public:
									CTutorial(class CInstructor* pInstructor, size_t iTutorial, int iPosition, int iWidth, bool bAutoNext, std::wstring sText);

public:
	class CInstructor*				m_pInstructor;
	size_t							m_iTutorial;
	int								m_iPosition;
	std::wstring					m_sText;
	int								m_iWidth;
	bool							m_bAutoNext;
};

class CTutorialPanel : public glgui::CPanel
{
public:
									CTutorialPanel(CTutorial* pTutorial);

public:
	virtual void					Paint(int x, int y, int w, int h);

	virtual bool					IsCursorListener() {return true;};
	virtual bool					MousePressed(int code, int mx, int my);

protected:
	CTutorial*						m_pTutorial;

	glgui::CLabel*					m_pText;
};

class CInstructor
{
public:
									CInstructor();
									~CInstructor();

public:
	void							Clear();
	void							Initialize();

	void							SetActive(bool bActive);
	bool							GetActive() { return m_bActive; };

	void							DisplayFirstTutorial();
	void							NextTutorial();

	void							DisplayTutorial(size_t iTutorial);
	void							HideTutorial();
	void							ShowTutorial();
	void							FinishedTutorial(size_t iTutorial);

	enum {
		POSITION_TOPCENTER,
		POSITION_POWERBARS,
		POSITION_ACTIVETANK,
		POSITION_TOPLEFT,
	};

	enum {
		TUTORIAL_INTRO,
		TUTORIAL_POWERPOINTS,
		TUTORIAL_MOVE,
		TUTORIAL_TURN,
		TUTORIAL_AIM,
		TUTORIAL_POWER,
		TUTORIAL_UPGRADE,
		TUTORIAL_KEYS,
	};

protected:
	bool							m_bActive;
	std::map<size_t, CTutorial*>	m_apTutorials;
	size_t							m_iCurrentTutorial;
	CTutorialPanel*					m_pCurrentPanel;
};

#endif