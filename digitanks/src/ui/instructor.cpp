#include "instructor.h"

#include <glgui/glgui.h>

#include "digitanks/digitank.h"
#include "digitanks/digitanksgame.h"
#include "digitankswindow.h"
#include "renderer/renderer.h"

using namespace glgui;

CInstructor::CInstructor()
{
	m_bActive = true;
	m_pCurrentPanel = NULL;
	Initialize();
}

CInstructor::~CInstructor()
{
	HideTutorial();
	Clear();
}

void CInstructor::Clear()
{
	for (size_t i = 0; i < m_apTutorials.size(); i++)
		delete m_apTutorials[i];
	m_apTutorials.clear();
}

void CInstructor::Initialize()
{
	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	Clear();

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_INTRO, new CTutorial(this, TUTORIAL_INTRO, POSITION_TOPCENTER, 200, true,
		L"Welcome to Digitanks!\n \nThis quick tutorial will help you get accustomed to the game.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWERPOINTS, new CTutorial(this, TUTORIAL_POWERPOINTS, POSITION_POWERBARS, 300, true,
		L"TANK POWER\n \nEach Digitank has a limited amount of Power which can be used for attack, defense, or movement.\n \nChoose how you use your Power carefully, because using your Power in one area means it can't be used in another. You can't save your power for the next turn, but any power used will be available again next turn.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_MOVE, new CTutorial(this, TUTORIAL_MOVE, POSITION_ACTIVETANK, 250, false,
		L"MOVE YOUR TANKS\n \nClick within the yellow area to move your tank. Don't move your tank too far, or you won't have enough Power for shooting and defense.\n \nIf you don't want to move your tank, press space bar to skip your move.")));

	//m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_TURN, new CTutorial(this, TUTORIAL_TURN, POSITION_ACTIVETANK, 200, false,
	//	L"TURN YOUR TANKS\n \nClick a spot to turn your tank. Your tank will aim its body directly at that spot. Turning your tank takes some movement power.\n \nClicking on your tank will cancel the move.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_AIM, new CTutorial(this, TUTORIAL_AIM, POSITION_TOPLEFT, 300, false,
		L"AIM YOUR TANKS\n \nClick a spot to aim your tank. At the end of your turn, your tank will fire on this spot.\n \nYour tank has very good accuracy inside the green area. Outside that area, accuracy will decrease. Your tank's maximum range is the red circle.\n \nIf you don't want to fire your tanks, you can press enter at any time to end your turn.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWER, new CTutorial(this, TUTORIAL_POWER, POSITION_ACTIVETANK, 200, true,
		L"POWER YOUR CANNON\n \nNow you can choose how to divide your remaining Power between your cannon and your shields. Don't forget to leave enough shields up to defend yourself!\n \nIf you have aimed your tank, you can press the 'Set Power' button to change your power selection.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_UPGRADE, new CTutorial(this, TUTORIAL_UPGRADE, POSITION_POWERBARS, 250, true,
		L"UPGRADE YOUR TANK\n \nYour tanks have some bonus points that you can use to upgrade their Power. Press the 'Promote' button to choose how to allocate your bonus points.\n \nPicking up powerups (those spinny prisms!) or killing other tanks can grant you additional bonus points.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_KEYS, new CTutorial(this, TUTORIAL_KEYS, POSITION_TOPLEFT, 200, false,
		L"ADDITIONAL KEYS\n \nHold Shift: Command all tanks\nSpacebar: Next tank\nTab: Next command\nAlt: Show Power/HP bars\nEnter: End turn\nRight mouse button: Drag camera\n \nYou can leave this window open for reference.\n \nEnjoy Digitanks!")));
}

void CInstructor::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!bActive)
		HideTutorial();
	else
	{
		Initialize();
		DisplayFirstTutorial();
	}
}

void CInstructor::DisplayFirstTutorial()
{
	m_iCurrentTutorial = TUTORIAL_INTRO;
	DisplayTutorial(m_iCurrentTutorial);
}

void CInstructor::NextTutorial()
{
	DisplayTutorial(++m_iCurrentTutorial);
}

void CInstructor::DisplayTutorial(size_t iTutorial)
{
	if (!m_bActive)
		return;

	if (m_apTutorials.find(iTutorial) == m_apTutorials.end())
	{
		SetActive(false);
		return;
	}

	// May not skip or go back tutorials!
	if (iTutorial < m_iCurrentTutorial || iTutorial > m_iCurrentTutorial+1)
		return;

	m_iCurrentTutorial = iTutorial;

	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = new CTutorialPanel(m_apTutorials[iTutorial]);
	glgui::CRootPanel::Get()->AddControl(m_pCurrentPanel, true);
}

void CInstructor::ShowTutorial()
{
	DisplayTutorial(m_iCurrentTutorial);
}

void CInstructor::HideTutorial()
{
	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
		m_pCurrentPanel = NULL;
	}
}

void CInstructor::FinishedTutorial(size_t iTutorial)
{
	if (iTutorial != m_iCurrentTutorial)
		return;

	if (m_pCurrentPanel)
	{
		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	if (m_apTutorials[iTutorial]->m_bAutoNext)
		m_apTutorials[iTutorial]->m_pInstructor->NextTutorial();
}

CTutorial::CTutorial(CInstructor* pInstructor, size_t iTutorial, int iPosition, int iWidth, bool bAutoNext, std::wstring sText)
{
	m_pInstructor = pInstructor;
	m_iTutorial = iTutorial;
	m_iPosition = iPosition;
	m_iWidth = iWidth;
	m_bAutoNext = bAutoNext;
	m_sText = sText;
}

CTutorialPanel::CTutorialPanel(CTutorial* pTutorial)
	: CPanel(0, 0, 100, 100)
{
	m_pTutorial = pTutorial;

	m_pText = new CLabel(0, 0, m_pTutorial->m_iWidth, 1000, "");
	m_pText->SetText(pTutorial->m_sText.c_str());
	m_pText->SetPos(10, 10);
	m_pText->SetSize(m_pTutorial->m_iWidth, (int)m_pText->GetTextHeight());
	m_pText->SetWrap(true);
	m_pText->SetAlign(CLabel::TA_MIDDLECENTER);
	AddControl(m_pText);

	SetSize(m_pText->GetWidth()+20, m_pText->GetHeight()+20);

	switch (pTutorial->m_iPosition)
	{
	case CInstructor::POSITION_TOPCENTER:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2, 100);
		break;

	case CInstructor::POSITION_POWERBARS:
		SetPos(glgui::CRootPanel::Get()->GetWidth()/2-m_pTutorial->m_iWidth/2 + 50, glgui::CRootPanel::Get()->GetHeight()-160-GetHeight());
		break;

	case CInstructor::POSITION_ACTIVETANK:
	{
		CSelectable* pSelection = DigitanksGame()->GetCurrentSelection();

		Vector vecCurrentSelectable = Game()->GetRenderer()->ScreenPosition(pSelection->GetOrigin());

		SetPos((int)vecCurrentSelectable.x - GetWidth() - 100, (int)vecCurrentSelectable.y - 50);
		if (GetLeft() < 0)
			SetPos(0, GetTop());
		if (GetTop() < 0)
			SetPos(GetLeft(), 0);
		break;
	}

	case CInstructor::POSITION_TOPLEFT:
		SetPos(100, 100);
		break;
	}
}

void CTutorialPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h);

	CPanel::Paint(x, y, w, h);
}

bool CTutorialPanel::MousePressed(int code, int mx, int my)
{
	SetVisible(false);
	if (m_pTutorial->m_bAutoNext)
		m_pTutorial->m_pInstructor->NextTutorial();
	return true;
}
