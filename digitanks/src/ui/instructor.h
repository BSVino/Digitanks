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
									CTutorial(class CInstructor* pInstructor, eastl::string sTutorial, eastl::string sNextTutorial, int iPosition, int iWidth, bool bAutoNext, eastl::string16 sText);

public:
	class CInstructor*				m_pInstructor;
	eastl::string					m_sTutorialName;
	eastl::string					m_sNextTutorial;
	int								m_iPosition;
	eastl::string16					m_sText;
	int								m_iWidth;
	bool							m_bAutoNext;
	bool							m_bKillOnFinish;
	float							m_flSlideAmount;
	bool							m_bSlideX;

	eastl::string					m_sButton1Text;
	eastl::string					m_sButton1Action;

	eastl::string					m_sButton2Text;
	eastl::string					m_sButton2Action;
};

class CTutorialPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CTutorialPanel, glgui::CPanel);

public:
									CTutorialPanel(CTutorial* pTutorial);

public:
	virtual void					Paint(int x, int y, int w, int h);

	virtual bool					IsCursorListener() {return true;};
	virtual bool					MousePressed(int code, int mx, int my);

	EVENT_CALLBACK(CTutorialPanel,	Button1);
	EVENT_CALLBACK(CTutorialPanel,	Button2);

protected:
	CTutorial*						m_pTutorial;

	glgui::CLabel*					m_pText;

	glgui::CButton*					m_pButton1;
	glgui::CButton*					m_pButton2;

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

	void							ReadLesson(const class CData* pData);

	void							SetActive(bool bActive);
	bool							GetActive() { return m_bActive; };

	void							DisplayFirstTutorial(eastl::string sTutorial);
	void							NextTutorial();

	void							DisplayTutorial(eastl::string sTutorial);
	void							HideTutorial();
	void							ShowTutorial();
	void							FinishedTutorial(eastl::string sTutorial, bool bForceNext = false);

	CTutorial*						GetCurrentTutorial() { return m_apTutorials[m_sCurrentTutorial]; };

	disable_t						GetDisabledFeatures();
	bool							IsFeatureDisabled(disable_t eFeature);

	enum {
		POSITION_TOPCENTER,
		POSITION_TOPLEFT,
		POSITION_SCENETREE,
		POSITION_BUTTONS,
	};

protected:
	bool							m_bActive;
	eastl::map<eastl::string, CTutorial*>	m_apTutorials;
	eastl::string					m_sLastTutorial;
	eastl::string					m_sCurrentTutorial;
	CTutorialPanel*					m_pCurrentPanel;
};

#endif