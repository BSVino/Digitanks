#pragma once

#include <tmap.h>
#include <glgui/panel.h>

#include <textures/texturesheet.h>

typedef bool (*pfnConditionsMet)( class CPlayer *pPlayer, class CLesson *pLesson );
pfnConditionsMet Game_GetInstructorConditions(const tstring& sConditions);

class CLessonOutput
{
public:
	CLessonOutput()
	{
	};

	tstring    m_sOutput;
	tstring    m_sTarget;
	tstring    m_sInput;
	tstring    m_sArgs;
};

class CLesson
{
public:
	typedef enum
	{
		LEARN_DISPLAYING,
		LEARN_PERFORMING,
	} learn_t;

	typedef enum
	{
		LESSON_BUTTON,      // Tell the player about a button. Lower center display. Icon to display is interpreted as a button.
		LESSON_INFO,        // Tell the player some information. Lower center display. Icon is a regular texture.
		LESSON_ENVIRONMENT, // Point to the player something in his environment. Custom painting must be done.
	} lesson_t;

public:
	CLesson(class CInstructor* pInstructor, tstring sLesson);

public:
	class CInstructor*              m_pInstructor;
	tstring                         m_sLessonName;
	tstring                         m_sNextLesson;
	int                             m_iPosition;
	tstring                         m_sText;
	int                             m_iWidth;
	bool                            m_bAutoNext;
	bool                            m_bKillOnFinish;
	float                           m_flSlideAmount;
	bool                            m_bSlideX;

	int                             m_iPriority;
	tvector<tstring>                m_asPrerequisites;

	lesson_t                        m_iLessonType;
	learn_t                         m_iLearningMethod;
	int                             m_iTimesToLearn;

	pfnConditionsMet                m_pfnConditions;

	tvector<CLessonOutput>          m_aOutputs;
};

class CLessonPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CLessonPanel, glgui::CPanel);

public:
	CLessonPanel(CLesson* pLesson);

public:
	virtual void                    Paint(float x, float y, float w, float h);

	virtual bool                    IsCursorListener() {return true;};
	virtual bool                    MousePressed(int code, int mx, int my);

protected:
	CLesson*                        m_pLesson;

	glgui::CControl<glgui::CLabel>  m_pText;

	double                          m_flStartTime;
	bool                            m_bDoneScrolling;
};

class CInstructor
{
	friend class CLessonPanel;

public:
	CInstructor();
	~CInstructor();

public:
	void            Clear();
	void            Initialize();
	bool            IsInitialized() { return !!m_apLessons.size(); }

	void            ReadLesson(const class CData* pData);
	void            ReadLessonOutput(const class CData* pData, CLesson* pLesson);

	const tmap<tstring, CLesson*>& GetLessons() const { return m_apLessons; }
	CLesson*        GetLesson(const tstring& sLesson) { return m_apLessons[sLesson]; };

	void            SetActive(bool bActive);
	bool            GetActive() { return m_bActive; };

	void            DisplayFirstLesson(tstring sLesson);
	void            NextLesson();

	void            DisplayLesson(tstring sLesson);
	void            HideLesson();
	void            ShowLesson();
	void            FinishedLesson(tstring sLesson, bool bForceNext = false);

	CLesson*        GetCurrentLesson() { return m_apLessons[m_sCurrentLesson]; };
	CLessonPanel*   GetCurrentPanel() { return m_pCurrentPanel; };

	void            CallOutput(const tstring& sOutput);

	static pfnConditionsMet GetBaseConditions(const tstring& sCondition);

	enum {
		POSITION_TOPCENTER,
		POSITION_TOPLEFT,
		POSITION_LEFT,
	};

protected:
	bool                        m_bActive;
	tmap<tstring, CLesson*>     m_apLessons;
	tstring                     m_sLastLesson;
	tstring                     m_sCurrentLesson;
	CLessonPanel*               m_pCurrentPanel;
};
