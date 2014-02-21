#include "instructor.h"

#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <tinker/cvar.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>

#include <game/entities/game.h>
#include <game/entities/character.h>
#include <renderer/renderer.h>
#include <sound/sound.h>
#include <tengine/ui/gamewindow.h>

using namespace glgui;

CInstructor::CInstructor()
{
	m_bActive = true;
	m_pCurrentPanel = NULL;
	m_sLastLesson = "";
	Initialize();

//	CSoundLibrary::Get()->AddSound("sound/lesson-learned.wav");
}

CInstructor::~CInstructor()
{
	HideLesson();
	Clear();
}

void CInstructor::Clear()
{
	for (tmap<tstring, CLesson*>::iterator it = m_apLessons.begin(); it != m_apLessons.end(); it++)
		delete m_apLessons[it->first];
	m_apLessons.clear();
}

void CInstructor::Initialize()
{
	if (m_pCurrentPanel)
	{
		RootPanel()->RemoveControl(m_pCurrentPanel);
	}

	m_pCurrentPanel = NULL;

	Clear();

	std::basic_ifstream<tchar> f("scripts/instructor.txt");

	if (!f.is_open())
		return;

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Lesson")
			ReadLesson(pChildData);
	}
}

void CInstructor::ReadLesson(const class CData* pData)
{
	tstring sLessonName = pData->GetValueString();
	if (!sLessonName.length())
	{
		TError("Found a lesson with no name.\n");
		return;
	}

	CLesson* pLesson = new CLesson(this, sLessonName);
	m_apLessons[sLessonName] = pLesson;

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Position")
		{
			tstring sPosition = pChildData->GetValueString();
			if (sPosition == "top-center")
				pLesson->m_iPosition = POSITION_TOPCENTER;
			else if (sPosition == "top-left")
				pLesson->m_iPosition = POSITION_TOPLEFT;
			else if (sPosition == "left")
				pLesson->m_iPosition = POSITION_LEFT;
		}
		else if (pChildData->GetKey() == "Width")
			pLesson->m_iWidth = pChildData->GetValueInt();
		else if (pChildData->GetKey() == "Next")
			pLesson->m_sNextLesson = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Text")
			pLesson->m_sText = pChildData->GetValueString();
		else if (pChildData->GetKey() == "SlideAmount")
			pLesson->m_flSlideAmount = pChildData->GetValueFloat();
		else if (pChildData->GetKey() == "SlideX")
			pLesson->m_bSlideX = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Output")
			ReadLessonOutput(pChildData, pLesson);
		else if (pChildData->GetKey() == "Priority")
			pLesson->m_iPriority = pChildData->GetValueInt();
		else if (pChildData->GetKey() == "LessonType")
		{
			tstring sLessonType = pChildData->GetValueString();
			if (sLessonType == "button")
				pLesson->m_iLessonType = CLesson::LESSON_BUTTON;
			else if (sLessonType == "info")
				pLesson->m_iLessonType = CLesson::LESSON_INFO;
			else if (sLessonType == "environment")
				pLesson->m_iLessonType = CLesson::LESSON_ENVIRONMENT;
		}
		else if (pChildData->GetKey() == "LearningMethod")
		{
			tstring sLearningMethod = pChildData->GetValueString();
			if (sLearningMethod == "displaying")
				pLesson->m_iLearningMethod = CLesson::LEARN_DISPLAYING;
			else if (sLearningMethod == "display")
				pLesson->m_iLearningMethod = CLesson::LEARN_DISPLAYING;
			else if (sLearningMethod == "performing")
				pLesson->m_iLearningMethod = CLesson::LEARN_PERFORMING;
			else if (sLearningMethod == "perform")
				pLesson->m_iLearningMethod = CLesson::LEARN_PERFORMING;
		}
		else if (pChildData->GetKey() == "TimesToLearn")
			pLesson->m_iTimesToLearn = pChildData->GetValueInt();
		else if (pChildData->GetKey() == "Conditions")
			pLesson->m_pfnConditions = Game_GetInstructorConditions(pChildData->GetValueString()); // A dirty hack, but not a scary one.
	}
}

bool WhoCaresConditions( CPlayer *pPlayer, class CLesson *pLesson )
{
	return true;
}

bool ValidPlayerConditions( CPlayer *pPlayer, class CLesson *pLesson )
{
	if (pPlayer->GetCharacter())
		return true;

	return false;
}

bool ValidPlayerAliveConditions( CPlayer *pPlayer, class CLesson *pLesson )
{
	if (!ValidPlayerConditions(pPlayer, pLesson))
		return false;

	if (!pPlayer->IsAlive())
		return false;

	if (pPlayer->GetCharacter()->GetLastSpawn() >= 0 && GameServer()->GetGameTime() < pPlayer->GetCharacter()->GetLastSpawn() + 2)
		return false;

	return true;
}

bool ValidPlayerDeadConditions( CPlayer *pPlayer, class CLesson *pLesson )
{
	if (!ValidPlayerConditions(pPlayer, pLesson))
		return false;

	if (pPlayer->IsAlive())
		return false;

	return true;
}

pfnConditionsMet CInstructor::GetBaseConditions(const tstring& sConditions)
{
	if (sConditions == "WhoCares")
		return WhoCaresConditions;
	else if (sConditions == "ValidPlayer")
		return ValidPlayerConditions;
	else if (sConditions == "ValidPlayerAlive")
		return ValidPlayerAliveConditions;
	else if (sConditions == "ValidPlayerDead")
		return ValidPlayerDeadConditions;

	TAssert(false);
	TError("Couldn't find lesson condition '" + sConditions + "'\n");

	return nullptr;
}

void CInstructor::ReadLessonOutput(const CData* pData, CLesson* pLesson)
{
	CLessonOutput* pOutput = &pLesson->m_aOutputs.push_back();
	pOutput->m_sOutput = pData->GetValueString();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Target")
			pOutput->m_sTarget = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Input")
			pOutput->m_sInput = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Args")
			pOutput->m_sArgs = pChildData->GetValueString();
	}
}

void CInstructor::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!bActive)
		HideLesson();
}

void CInstructor::DisplayFirstLesson(tstring sLesson)
{
	m_bActive = true;
	m_sLastLesson = "";
	m_sCurrentLesson = sLesson;
	DisplayLesson(m_sCurrentLesson);
}

void CInstructor::NextLesson()
{
	CLesson* pLesson = GetCurrentLesson();
	if (!pLesson)
		return;

	DisplayLesson(pLesson->m_sNextLesson);
}

CVar lesson_enable("lesson_enable", "1");

void CInstructor::DisplayLesson(tstring sLesson)
{
	if (!lesson_enable.GetBool())
		return;

	if (!m_bActive)
		return;

	if (sLesson.length() == 0 || m_apLessons.find(sLesson) == m_apLessons.end())
	{
		if (m_apLessons[m_sCurrentLesson] && m_apLessons[m_sCurrentLesson]->m_bKillOnFinish)
			SetActive(false);

		if (m_pCurrentPanel)
			HideLesson();

		return;
	}

	if (m_pCurrentPanel)
		HideLesson();

	m_sCurrentLesson = sLesson;

	if (Game() && m_sLastLesson != m_sCurrentLesson)
		Game()->OnDisplayLesson(sLesson);

	m_sLastLesson = m_sCurrentLesson;

	CLesson* pLesson = m_apLessons[sLesson];

	CPlayer *pLocalPlayer = Game()->GetLocalPlayer();
	if (pLesson->m_iLearningMethod == CLesson::LEARN_DISPLAYING)
		pLocalPlayer->Instructor_LessonLearned(sLesson);

	m_pCurrentPanel = new CLessonPanel(pLesson);
	RootPanel()->AddControl(m_pCurrentPanel, true);

	CallOutput("OnDisplay");
}

void CInstructor::ShowLesson()
{
	DisplayLesson(m_sCurrentLesson);
}

void CInstructor::HideLesson()
{
	if (m_pCurrentPanel)
	{
		RootPanel()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel = NULL;

		CallOutput("OnClose");
	}
}

void CInstructor::FinishedLesson(tstring sLesson, bool bForceNext)
{
	if (sLesson != m_sCurrentLesson)
		return;

	if (m_pCurrentPanel)
		// Only play the sound if the current panel is showing so we don't play it multiple times.
		CSoundLibrary::PlaySound(NULL, "sound/lesson-learned.wav");

	if (m_apLessons[sLesson]->m_bAutoNext || bForceNext)
		NextLesson();
	else
		HideLesson();

	// If we get to the end here then we turn off the instructor as we have finished completely.
	if (GetCurrentLesson() && GetCurrentLesson()->m_bKillOnFinish)
		SetActive(false);
}

void CInstructor::CallOutput(const tstring& sOutput)
{
	if (!GetCurrentLesson())
		return;

	for (size_t i = 0; i < GetCurrentLesson()->m_aOutputs.size(); i++)
	{
		CLessonOutput* pOutput = &GetCurrentLesson()->m_aOutputs[i];
		if (pOutput->m_sOutput == sOutput)
		{
			for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
			{
				CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

				if (!pEntity)
					continue;

				if (pEntity->IsDeleted())
					continue;

				if (pOutput->m_sTarget.length() == 0)
					continue;

				if (pOutput->m_sTarget.length() == 0)
					continue;

				if (pOutput->m_sTarget[0] == '*')
				{
					if (tstring(pEntity->GetClassName()) != pOutput->m_sTarget.c_str()+1)
						continue;
				}
				else
				{
					if (pEntity->GetName() != pOutput->m_sTarget)
						continue;
				}

				pEntity->CallInput(pOutput->m_sInput, convertstring<char, tchar>(pOutput->m_sArgs));
			}
		}
	}
}

static CVar lesson_nexttime("lesson_nexttime", "20");	// Time between hints
static CVar lesson_downtime("lesson_downtime", "70");	// Time for three other hints in the intervening time.
static CVar lesson_learntime("lesson_learntime", "15");	// Time before a lesson can be learned again.
static CVar lesson_debug("lesson_debug", "0");
static CVar lesson_time("lesson_time", "8");

void CPlayer::Instructor_Initialize()
{
	m_flLastLesson = 0;

	m_apLessonPriorities.clear();
}

void CPlayer::Instructor_Respawn()
{
	if (!GameWindow()->GetInstructor()->IsInitialized())
		GameWindow()->GetInstructor()->Initialize();

	for (auto it = m_apLessonProgress.begin(); it != m_apLessonProgress.end(); it++)
		it->second.m_flLastTimeLearned = 0;

	m_flLastLesson = 0;
}

typedef CPlayer::CLessonProgress* LessonProgressPointer;
bool LessonPriorityCompare( const LessonProgressPointer& l, const LessonProgressPointer& r )
{
	//CPlayer* pPlayer = Game()->GetLocalPlayer();
	//if (pPlayer->m_flLastEnemySeen && gpGlobals->curtime < pPlayer->m_flLastEnemySeen + 3 && lhs->m_bPreferNoEnemies != rhs->m_bPreferNoEnemies)
	//	return !lhs->m_bPreferNoEnemies;

	CLesson* pLessonL = GameWindow()->GetInstructor()->GetLesson(l->m_sLessonName);
	CLesson* pLessonR = GameWindow()->GetInstructor()->GetLesson(r->m_sLessonName);

	// If two lessons are the same priority, use the one that was taught the most amount of time ago.
	if (pLessonL->m_iPriority == pLessonR->m_iPriority)
		return l->m_flLastTimeShowed > r->m_flLastTimeShowed;

	return ( pLessonL->m_iPriority > pLessonR->m_iPriority );
}

void CPlayer::Instructor_Think()
{
	if (!GameWindow()->GetInstructor()->IsInitialized())
		GameWindow()->GetInstructor()->Initialize();

	if (!m_apLessonProgress.size() && GameWindow()->GetInstructor()->GetLessons().size())
	{
		for (auto it = GameWindow()->GetInstructor()->GetLessons().begin(); it != GameWindow()->GetInstructor()->GetLessons().end(); it++)
		{
			CLessonProgress& oProgress = m_apLessonProgress[it->first];
			oProgress.m_sLessonName = it->first;
		}
	}

	if (m_flLastLesson < 0 || GameServer()->GetGameTime() > m_flLastLesson + lesson_nexttime.GetFloat())
	{
		m_apLessonPriorities.clear();

		for (auto it = m_apLessonProgress.begin(); it != m_apLessonProgress.end(); it++)
		{
			CLessonProgress* pLessonProgress = &it->second;
			CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(it->first);

			if (pLesson->m_iLessonType == CLesson::LESSON_ENVIRONMENT)
				continue;

			if (!Instructor_IsLessonValid(pLessonProgress))
				continue;

			m_apLessonPriorities.push_back(pLessonProgress);
			push_heap(m_apLessonPriorities.begin(), m_apLessonPriorities.end(), LessonPriorityCompare);
		}

		if (lesson_debug.GetBool() && m_apLessonPriorities.size())
		{
			TMsg("Instructor: Lesson priorities:\n");
			for (size_t j = 0; j < m_apLessonPriorities.size(); j++)
			{
				CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(m_apLessonPriorities[j]->m_sLessonName);
				TMsg(sprintf(" %d - " + m_apLessonPriorities[j]->m_sLessonName + " - %d\n", j+1, pLesson->m_iPriority));
			}
		}

		CLessonProgress* pBestLesson = Instructor_GetBestLesson();

		if (pBestLesson)
		{
			if (lesson_debug.GetBool())
			{
				CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(pBestLesson->m_sLessonName);
				TMsg(sprintf("Instructor: New lesson: " + pBestLesson->m_sLessonName + " Priority: %d\n", pLesson->m_iPriority));
			}

			m_flLastLesson = GameServer()->GetGameTime();
			pBestLesson->m_flLastTimeShowed = GameServer()->GetGameTime();

			GameWindow()->GetInstructor()->DisplayLesson(pBestLesson->m_sLessonName);
		}
	}
}

void CPlayer::Instructor_LessonLearned(const tstring& sLesson)
{
	if (!GameWindow()->GetInstructor()->IsInitialized())
		GameWindow()->GetInstructor()->Initialize();

	auto it = m_apLessonProgress.find(sLesson);
	TAssert(it != m_apLessonProgress.end());
	if (it == m_apLessonProgress.end())
		return;

	CLessonProgress* pLessonProgress = &it->second;

	TAssert(pLessonProgress);
	if (!pLessonProgress)
		return;

	// Can only learn a lesson once in a while, to ensure that it is truly learned.
	// The idea is that the player spends a couple seconds toying around with the
	// new feature, but won't spend all of the lessons in that time.
	if (GameServer()->GetGameTime() < pLessonProgress->m_flLastTimeLearned + lesson_learntime.GetFloat())
		return;

	pLessonProgress->m_flLastTimeLearned = GameServer()->GetGameTime();
	pLessonProgress->m_iTimesLearned++;

	if (lesson_debug.GetBool())
	{
		CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(sLesson);

		if (pLessonProgress->m_iTimesLearned < pLesson->m_iTimesToLearn)
			TMsg(sprintf("Instructor: Trained lesson " + sLesson + " - %d/%d\n", pLessonProgress->m_iTimesLearned, pLesson->m_iTimesToLearn));
		else if (pLessonProgress->m_iTimesLearned == pLesson->m_iTimesToLearn)
			TMsg("Instructor: Learned lesson " + sLesson + "\n");
	}
}

bool CPlayer::Instructor_IsLessonLearned(const CLessonProgress* pLessonProgress)
{
	TAssert(pLessonProgress);
	if (!pLessonProgress)
		return true;

	CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(pLessonProgress->m_sLessonName);

	TAssert(pLesson);
	if (!pLesson)
		return true;

	return pLessonProgress->m_iTimesLearned >= pLesson->m_iTimesToLearn;
}

// Can this lesson be displayed right now?
bool CPlayer::Instructor_IsLessonValid(const CLessonProgress* pLessonProgress)
{
	TAssert(pLessonProgress);
	if (!pLessonProgress)
		return true;

	CLesson* pLesson = GameWindow()->GetInstructor()->GetLesson(pLessonProgress->m_sLessonName);

	TAssert(pLesson);
	if (!pLesson)
		return true;

	if (Instructor_IsLessonLearned(pLessonProgress))
		return false;

	if (pLessonProgress->m_flLastTimeShowed != 0 && GameServer()->GetGameTime() < pLessonProgress->m_flLastTimeShowed + lesson_downtime.GetFloat())
		return false;

	for (size_t i = 0; i < pLesson->m_asPrerequisites.size(); i++)
	{
		if (!Instructor_IsLessonLearned(&m_apLessonProgress[pLesson->m_asPrerequisites[i]]))
			return false;
	}

	if (pLesson->m_pfnConditions)
		return pLesson->m_pfnConditions(this, pLesson);
	else
		return true;
}

CPlayer::CLessonProgress* CPlayer::Instructor_GetBestLesson()
{
	if (!m_apLessonPriorities.size())
		return nullptr;

	return m_apLessonPriorities[0];
}

void Lessons_Reset(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	CPlayer* pPlayer = Game()->GetLocalPlayer();
	pPlayer->Instructor_Initialize();
}

static CCommand lesson_reset("lesson_reset", Lessons_Reset);

CLesson::CLesson(CInstructor* pInstructor, tstring sLesson)
{
	m_pInstructor = pInstructor;
	m_sLessonName = sLesson;
	m_iPosition = CInstructor::POSITION_TOPCENTER;
	m_iWidth = 200;
	m_bAutoNext = true;
	m_bKillOnFinish = false;
	m_flSlideAmount = 0;
	m_bSlideX = true;
	m_iPriority = 0;
	m_iLessonType = LESSON_INFO;
	m_iLearningMethod = LEARN_DISPLAYING;
	m_iTimesToLearn = 3;
	m_pfnConditions = WhoCaresConditions;
}

CLessonPanel::CLessonPanel(CLesson* pLesson)
	: CPanel(0, 0, 100, 100)
{
	m_pLesson = pLesson;

	m_pText = AddControl(new CLabel(0, 0, (float)m_pLesson->m_iWidth, 1000, ""));
	m_pText->SetText(pLesson->m_sText.replace("\\n", "\n"));
	m_pText->SetPos(10, 0);
	m_pText->SetSize((float)m_pLesson->m_iWidth, m_pText->GetTextHeight()+10);
	m_pText->SetWrap(true);
	m_pText->SetAlign(CLabel::TA_MIDDLECENTER);
	m_pText->SetFont("text");

	m_pText->ComputeLines();

	SetSize(m_pText->GetWidth()+20, m_pText->GetHeight());

	switch (pLesson->m_iPosition)
	{
	case CInstructor::POSITION_TOPCENTER:
		SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, 100);
		break;

	case CInstructor::POSITION_TOPLEFT:
		SetPos(100, 100);
		break;

	case CInstructor::POSITION_LEFT:
		SetPos(100, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);
		break;
	}

	if (GameServer())
		m_flStartTime = GameServer()->GetGameTime();
	else
		m_flStartTime = 0;

	m_bDoneScrolling = true;
}

void CLessonPanel::Paint(float x, float y, float w, float h)
{
	if (m_flStartTime < 0 || GameServer()->GetGameTime() > m_flStartTime + lesson_time.GetFloat())
		return;

	if (m_pLesson->m_flSlideAmount > 0)
	{
		if (m_pLesson->m_bSlideX)
			x += Bias(RemapValClamped((float)(GameServer()->GetGameTime() - m_flStartTime), 0, 1, 1.0f, 0.0f), 0.2f) * m_pLesson->m_flSlideAmount;
		else
			y += Bias(RemapValClamped((float)(GameServer()->GetGameTime() - m_flStartTime), 0, 1, 1.0f, 0.0f), 0.2f) * m_pLesson->m_flSlideAmount;
	}

	bool bScrolling = false;
	if (!m_bDoneScrolling)
	{
		int iPrintChars = (int)((GameServer()->GetGameTime() - m_flStartTime)*70);
		m_pText->SetPrintChars(iPrintChars);

		bScrolling = (iPrintChars < (int)m_pText->GetText().length());

		if (!bScrolling)
			m_bDoneScrolling = true;
	}
	else
		m_pText->SetPrintChars(-1);

	CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 50), 1);

	CPanel::Paint(x, y, w, h);
}

bool CLessonPanel::MousePressed(int code, int mx, int my)
{
	if (BaseClass::MousePressed(code, mx, my))
		return true;

	if (!m_bDoneScrolling)
	{
		m_bDoneScrolling = true;
		return true;
	}

	return false;
}
