#include "instructor.h"

#include <glgui/glgui.h>

#include "digitanks/digitank.h"
#include "digitanks/digitanksgame.h"
#include "digitankswindow.h"
#include "renderer/renderer.h"
#include "sound/sound.h"

using namespace glgui;

CInstructor::CInstructor()
{
	m_bActive = true;
	m_pCurrentPanel = NULL;
	m_iLastTutorial = -1;
	Initialize();

	CSoundLibrary::Get()->AddSound("sound/lesson-learned.wav");
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
		L"Welcome to Digitanks!\n \nThis tutorial will help you get accustomed to the game.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_SELECTION, new CTutorial(this, TUTORIAL_SELECTION, POSITION_TOPCENTER, 200, true,
		L"TANK SELECTION\n \nThis is your tank. Select it by clicking on it with the left mouse button.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_MOVECAMERA, new CTutorial(this, TUTORIAL_MOVECAMERA, POSITION_TOPCENTER, 200, true,
		L"VIEW CONTROLS\n \nFirst, let's take a look around. Left click on open terrain to re-center the view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_TURNCAMERA, new CTutorial(this, TUTORIAL_TURNCAMERA, POSITION_TOPCENTER, 200, true,
		L"VIEW CONTROLS\n \nHold down the left mouse button and drag to rotate the view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_ZOOMCAMERA, new CTutorial(this, TUTORIAL_ZOOMCAMERA, POSITION_TOPCENTER, 200, true,
		L"VIEW CONTROLS\n \nYou can use the mouse wheel to zoom in and out.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_MOVE, new CTutorial(this, TUTORIAL_MOVE, POSITION_TOPLEFT, 300, true,
		L"MOVE YOUR TANK\n \nAn enemy tank is closeby. You are now in 'move mode' and can move your tank closer to get a better shot. You can only move once per turn. Right click inside the yellow area to move your tank.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_AIM, new CTutorial(this, TUTORIAL_AIM, POSITION_TOPLEFT, 200, true,
		L"AIM YOUR TANK\n \nYou're closing in for the kill! Now aim at the enemy tank by right clicking on him.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_RANGE, new CTutorial(this, TUTORIAL_RANGE, POSITION_TOPLEFT, 300, true,
		L"TANK RANGE\n \nYour tank has very good accuracy inside the green area, but outside that your accuracy will decrease. Your tank's maximum range is the red circle.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_BUTTONS, new CTutorial(this, TUTORIAL_BUTTONS, POSITION_TOPLEFT, 200, true,
		L"CONTROL BUTTONS\n \nYou can enter move mode and aim mode at any time by using the buttons on the lower right of the screen.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWER, new CTutorial(this, TUTORIAL_POWER, POSITION_TOPLEFT, 300, true,
		L"ENERGIZE YOUR CANNON\n \nNow choose how much Energy you want to use on your attack. Press the 'Set Energy' button and the Energy sliders will appear next to your tank. Move the slider to the desired level and click the left mouse button to select a setting.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWERPOINTS, new CTutorial(this, TUTORIAL_POWERPOINTS, POSITION_TOPCENTER, 300, true,
		L"TANK ENERGY\n \nEach Digitank has a limited amount of Energy which can be used for attack, defense, or movement. The bars at the bottom of the screen show the selected tank's Energy.\n \nChoose how you use your Energy carefully, because using your Energy in one area means it can't be used in another. You can't save your Energy for the next turn, but any Energy used will be available again next turn.\n \nClick here to continue.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_ENTERKEY, new CTutorial(this, TUTORIAL_ENTERKEY, POSITION_TOPCENTER, 200, false,
		L"END YOUR TURN\n \nNow press the 'Enter' key to end your turn and fire your tank.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_TURN, new CTutorial(this, TUTORIAL_TURN, POSITION_TOPLEFT, 300, true,
		L"TURN YOUR TANK\n \nGood job, you messed that guy up pretty good! However, his return fire damaged your shields. You can rotate your tank to bring another shield to bear. Turning your tank takes some Energy. Click the 'Turn' button and right click a spot to turn your tank. Your tank will aim its body directly at that spot.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_FINISHHIM, new CTutorial(this, TUTORIAL_FINISHHIM, POSITION_TOPLEFT, 200, false,
		L"FINISH THE JOB\n \nNow let's finish the job. Aim your tank at the enemy again and press enter to end your turn and fire. If you have trouble hitting the tank, try rotating your camera for a better view.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_UPGRADE, new CTutorial(this, TUTORIAL_UPGRADE, POSITION_TOPCENTER, 250, true,
		L"UPGRADE YOUR TANK\n \nYou destroyed the enemy tank, and received a promotion! Your promotion gives you bonus points that you can use to upgrade your tank's Energy. Press the 'Promote' button to show a list of upgrade options, and then select an upgrade.\n \nKilling enemy tanks can grant you additional bonus points.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_POWERUP, new CTutorial(this, TUTORIAL_POWERUP, POSITION_TOPLEFT, 250, true,
		L"GET THE POWERUP\n \nLook at that! A powerup has appeared next to your tank. Move your tank on top of the powerup and it will turn green. Keep the tank in this position and you'll be able to use the bonus points next turn. Picking up more powerups can grant you additional bonus points.\n \nPress the 'Enter' key to end your turn and pick up the powerup.")));

	m_apTutorials.insert(std::pair<size_t, CTutorial*>(TUTORIAL_THEEND, new CTutorial(this, TUTORIAL_THEEND, POSITION_TOPCENTER, 250, true,
		L"END OF TUTORIAL\n \nThat's it! You can start a new game by opening the menu with the 'Escape' key. Enjoy Digitanks!")));
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

	if (m_iLastTutorial != m_iCurrentTutorial)
		DigitanksGame()->OnDisplayTutorial(iTutorial);

	m_iLastTutorial = m_iCurrentTutorial;

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
		// Only play the sound if the current panel is showing so we don't play it multiple times.
		CSoundLibrary::PlaySound(NULL, "sound/lesson-learned.wav");

		CRootPanel::Get()->RemoveControl(m_pCurrentPanel);
		m_pCurrentPanel->Delete();
	}

	m_pCurrentPanel = NULL;

	if (m_apTutorials[iTutorial]->m_bAutoNext)
		m_apTutorials[iTutorial]->m_pInstructor->NextTutorial();
}

disable_t CInstructor::GetDisabledFeatures()
{
	int iDisabled = 0;

	if (GetCurrentTutorial() < TUTORIAL_TURNCAMERA)
		iDisabled |= DISABLE_ROTATE;

	if (GetCurrentTutorial() < TUTORIAL_ENTERKEY)
		iDisabled |= DISABLE_ENTER;

	return (disable_t)iDisabled;
}

bool CInstructor::IsFeatureDisabled(disable_t eFeature)
{
	if (!GetActive())
		return false;

	return !!(GetDisabledFeatures()&eFeature);
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

		if (!pSelection)
			break;

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
