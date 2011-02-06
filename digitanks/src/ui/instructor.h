#ifndef DT_INSTRUCTOR_H
#define DT_INSTRUCTOR_H

#include <EASTL/map.h>
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
									CTutorial(class CInstructor* pInstructor, size_t iTutorial, int iPosition, int iWidth, bool bAutoNext, eastl::string16 sText);

public:
	class CInstructor*				m_pInstructor;
	size_t							m_iTutorial;
	int								m_iPosition;
	eastl::string16					m_sText;
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

	float							m_flStartTime;
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
	void							DisplayFirstUnitsTutorial();
	void							DisplayIngameArtilleryTutorial();
	void							DisplayIngameStrategyTutorial();
	void							NextTutorial();

	void							DisplayTutorial(size_t iTutorial);
	void							HideTutorial();
	void							ShowTutorial();
	void							FinishedTutorial(size_t iTutorial, bool bForceNext = false);

	size_t							GetCurrentTutorial() { return m_iCurrentTutorial; };

	disable_t						GetDisabledFeatures();
	bool							IsFeatureDisabled(disable_t eFeature);

	enum {
		POSITION_TOPCENTER,
		POSITION_TOPLEFT,
		POSITION_SCENETREE,
		POSITION_BUTTONS,
	};

	enum {
		TUTORIAL_INTRO_BASICS,
		TUTORIAL_MOVECAMERA,
		TUTORIAL_MOVECAMERA2,
		TUTORIAL_TURNCAMERA,
		TUTORIAL_ZOOMCAMERA,
		TUTORIAL_SELECTION,
		TUTORIAL_MOVE,
		TUTORIAL_AIM,
		TUTORIAL_ATTACK,
		TUTORIAL_BUTTONS,
		TUTORIAL_ENERGY,
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
		TUTORIAL_FLEET_POINTS,
		TUTORIAL_THEEND_BASES,

		TUTORIAL_INTRO_UNITS,
		TUTORIAL_INFANTRY,
		TUTORIAL_FORTIFYING,
		TUTORIAL_FORTIFYING2,
		TUTORIAL_ARTILLERY,
		TUTORIAL_DEPLOYING,
		TUTORIAL_DEPLOYING2,
		TUTORIAL_FIRE_ARTILLERY,
		TUTORIAL_ARTILLERY_SHIELDS,
		TUTORIAL_ROGUE,
		TUTORIAL_TORPEDO,
		TUTORIAL_DISCONNECTING_SUPPLIES,
		TUTORIAL_THEEND_UNITS,

		TUTORIAL_INGAME_ARTILLERY_SELECT,
		TUTORIAL_INGAME_ARTILLERY_AIM,
		TUTORIAL_INGAME_ARTILLERY_CHOOSE_WEAPON,
		TUTORIAL_INGAME_ARTILLERY_COMMAND,

		TUTORIAL_INGAME_STRATEGY_SELECT,
		TUTORIAL_INGAME_STRATEGY_COMMAND,
		TUTORIAL_INGAME_STRATEGY_DEPLOY,
		TUTORIAL_INGAME_STRATEGY_BUILDBUFFER,
		TUTORIAL_INGAME_STRATEGY_PLACEBUFFER,
	};

protected:
	bool							m_bActive;
	eastl::map<size_t, CTutorial*>	m_apTutorials;
	size_t							m_iLastTutorial;
	size_t							m_iCurrentTutorial;
	CTutorialPanel*					m_pCurrentPanel;
};

#endif