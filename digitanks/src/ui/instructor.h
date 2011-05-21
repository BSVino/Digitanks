#ifndef DT_INSTRUCTOR_H
#define DT_INSTRUCTOR_H

#include <EASTL/map.h>
#include <glgui/glgui.h>

#include <models/texturesheet.h>

typedef enum
{
	DISABLE_NOTHING		= 0,
	DISABLE_VIEW_MOVE	= (1<<0),
	DISABLE_VIEW_ROTATE	= (1<<1),
	DISABLE_ENTER		= (1<<2),
	DISABLE_BUFFER		= (1<<3),
	DISABLE_PSU			= (1<<4),
	DISABLE_LOADERS		= (1<<5),
	DISABLE_SELECT		= (1<<6),
	DISABLE_HOWTOPLAY	= (1<<7),
} disable_t;

class CLessonOutput
{
public:
	CLessonOutput()
	{
	};

	eastl::string			m_sOutput;
	eastl::string			m_sTarget;
	eastl::string			m_sInput;
	eastl::string			m_sArgs;
};

class CTutorial
{
public:
									CTutorial(class CInstructor* pInstructor, eastl::string sTutorial, eastl::string sNextTutorial, int iPosition, int iWidth, bool bAutoNext, eastl::string16 sText);
									CTutorial(class CInstructor* pInstructor, eastl::string sTutorial);

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
	bool							m_bLeaveMouthOpen;
	int								m_iHintButton;
	bool							m_bMousePrompt;

	eastl::string					m_sButton1Text;
	eastl::string					m_sButton1Action;

	eastl::string					m_sButton2Text;
	eastl::string					m_sButton2Action;

	disable_t						m_eDisable;
	disable_t						m_eEnable;

	Vector2D						m_vecSetViewTarget;
	EAngle							m_angSetViewAngle;
	float							m_flSetViewDistance;

	eastl::string					m_sHelperEmotion;

	eastl::vector<CLessonOutput>	m_aOutputs;
};

class CTutorialPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CTutorialPanel, glgui::CPanel);

public:
									CTutorialPanel(CTutorial* pTutorial, bool bFirstHelperPanel = false);

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

	bool							m_bFirstHelperPanel;
};

class CInstructor
{
	friend class CTutorialPanel;

public:
									CInstructor();
									~CInstructor();

public:
	void							Clear();
	void							Initialize();

	void							ReadLesson(const class CData* pData);
	void							ReadLessonOutput(const class CData* pData, CTutorial* pLesson);

	void							SetActive(bool bActive);
	bool							GetActive() { return m_bActive; };

	void							DisplayFirstTutorial(eastl::string sTutorial);
	void							NextTutorial();

	void							DisplayTutorial(eastl::string sTutorial);
	void							HideTutorial();
	void							ShowTutorial();
	void							FinishedTutorial(eastl::string sTutorial, bool bForceNext = false);

	CTutorial*						GetCurrentTutorial() { return m_apTutorials[m_sCurrentTutorial]; };
	CTutorialPanel*					GetCurrentPanel() { return m_pCurrentPanel; };

	void							CallOutput(const eastl::string& sOutput);

	disable_t						GetDisabledFeatures();
	bool							IsFeatureDisabled(disable_t eFeature);

	enum {
		POSITION_TOPCENTER,
		POSITION_TOPLEFT,
		POSITION_SCENETREE,
		POSITION_BUTTONS,
	};

	const CTextureSheet*			GetEmotionsSheet() { return &m_EmotionsSheet; }
	const CTextureSheet*			GetEmotionsOpenSheet() { return &m_EmotionsOpenSheet; }

protected:
	bool							m_bActive;
	eastl::map<eastl::string, CTutorial*>	m_apTutorials;
	eastl::string					m_sLastTutorial;
	eastl::string					m_sCurrentTutorial;
	CTutorialPanel*					m_pCurrentPanel;
	bool							m_bHelperSpeaking;

	disable_t						m_eDisabled;

	CTextureSheet					m_EmotionsSheet;
	CTextureSheet					m_EmotionsOpenSheet;
};

#endif