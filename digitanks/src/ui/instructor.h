#ifndef DT_INSTRUCTOR_H
#define DT_INSTRUCTOR_H

#include <map>
#include <glgui/glgui.h>

typedef enum
{
	DISABLE_ROTATE = (1<<0),
	DISABLE_ENTER = (1<<1),
	DISABLE_BUFFER = (1<<2),
	DISABLE_PSU = (1<<3),
	DISABLE_LOADERS = (1<<4),
} disable_t;

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

	void							DisplayFirstBasicsTutorial();
	void							DisplayFirstBasesTutorial();
	void							NextTutorial();

	void							DisplayTutorial(size_t iTutorial);
	void							HideTutorial();
	void							ShowTutorial();
	void							FinishedTutorial(size_t iTutorial);

	size_t							GetCurrentTutorial() { return m_iCurrentTutorial; };

	disable_t						GetDisabledFeatures();
	bool							IsFeatureDisabled(disable_t eFeature);

	enum {
		POSITION_TOPCENTER,
		POSITION_POWERBARS,
		POSITION_ACTIVETANK,
		POSITION_TOPLEFT,
	};

	enum {
		TUTORIAL_INTRO_BASICS,
		TUTORIAL_MOVECAMERA,
		TUTORIAL_TURNCAMERA,
		TUTORIAL_ZOOMCAMERA,
		TUTORIAL_SELECTION,
		TUTORIAL_MOVE,
		TUTORIAL_AIM,
		TUTORIAL_ATTACK,
		TUTORIAL_BUTTONS,
		TUTORIAL_ENERGY,
		TUTORIAL_POWERPOINTS,
		TUTORIAL_ENTERKEY,
		TUTORIAL_TURN,
		TUTORIAL_FINISHHIM,
		TUTORIAL_UPGRADE,
		TUTORIAL_POWERUP,
		TUTORIAL_SHIFTSELECT,
		TUTORIAL_BOXSELECT,
		TUTORIAL_THEEND_BASICS,

		TUTORIAL_INTRO_BASES,
		TUTORIAL_CPU,
		TUTORIAL_BUFFER,
		TUTORIAL_POWER,
		TUTORIAL_NETWORK,
		TUTORIAL_PSU,
		TUTORIAL_SUPPLY,
		TUTORIAL_LOADER,
		TUTORIAL_EFFICIENCY,
		TUTORIAL_PRODUCING_UNITS,
		TUTORIAL_LIMITED_POWER,
		TUTORIAL_FLEET_POINTS,
		TUTORIAL_THEEND_BASES,
	};

protected:
	bool							m_bActive;
	std::map<size_t, CTutorial*>	m_apTutorials;
	size_t							m_iLastTutorial;
	size_t							m_iCurrentTutorial;
	CTutorialPanel*					m_pCurrentPanel;
};

#endif