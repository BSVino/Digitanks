#include "console.h"

#include <platform.h>
#include <strutils.h>

#include "application.h"
#include "keys.h"
#include "cvar.h"

#ifdef _DEBUG
#define DEV_VALUE "1"
#else
#define DEV_VALUE "0"
#endif

CVar developer("developer", DEV_VALUE);

#undef DEV_VALUE

CConsole::CConsole()
	: glgui::CPanel(0, 0, 100, 100)
{
	glgui::CRootPanel::Get()->AddControl(this, true);

	m_pOutput = new glgui::CLabel(0, 0, 100, 100, L"");
	m_pOutput->SetAlign(glgui::CLabel::TA_BOTTOMLEFT);
	AddControl(m_pOutput);

	m_pInput = new glgui::CTextField();
	AddControl(m_pInput);

	m_bBackground = true;
}

void CConsole::Destructor()
{
	glgui::CRootPanel::Get()->RemoveControl(this);

	BaseClass::Destructor();
}

bool CConsole::IsVisible()
{
	if (developer.GetBool() && m_bBackground)
		return true;

	return BaseClass::IsVisible();
}

void CConsole::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	m_pInput->SetFocus(bVisible);
}

bool CConsole::IsOpen()
{
	return BaseClass::IsVisible();
}

bool CConsole::IsCursorListener()
{
	// Don't interfere with mouse events, we're just showing an overlay.
	if (developer.GetBool() && !BaseClass::IsVisible())
		return false;

	return BaseClass::IsCursorListener();
}

void CConsole::Layout()
{
	SetSize(glgui::CRootPanel::Get()->GetWidth()/3, glgui::CRootPanel::Get()->GetHeight()-150);
	SetPos(glgui::CRootPanel::Get()->GetWidth()/6, 0);

	m_pInput->SetSize(GetWidth(), 20);
	m_pInput->SetPos(0, GetHeight()-20);

	m_pOutput->SetSize(GetWidth(), GetHeight()-24);
	m_pOutput->SetPos(0, 0);

	BaseClass::Layout();
}

void CConsole::Paint(int x, int y, int w, int h)
{
	if (!CApplication::Get()->IsOpen())
		return;

	if (!BaseClass::IsVisible() && developer.GetBool())
	{
		int iAlpha = m_pOutput->GetAlpha();
		m_pOutput->SetAlpha(100);
		m_pOutput->Paint();
		m_pOutput->SetAlpha(iAlpha);
		return;
	}

	if (!BaseClass::IsVisible())
		return;

	glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 200));

	BaseClass::Paint(x, y, w, h);

	eastl::string16 sInput = m_pInput->GetText();
	if (sInput.length() && sInput.find(L' ') == ~0)
	{
		eastl::vector<eastl::string16> sCommands = CCommand::GetCommandsBeginningWith(sInput);

		size_t iCommandsToShow = sCommands.size();
		bool bAbbreviated = false;

		if (iCommandsToShow > 5)
		{
			iCommandsToShow = 5;
			bAbbreviated = true;
		}

		if (bAbbreviated)
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, (iCommandsToShow+1)*13+3, Color(0, 0, 0, 200));
		else
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, iCommandsToShow*13+3, Color(0, 0, 0, 200));

		int iCommandsPainted = 0;
		for (size_t i = 0; i < iCommandsToShow; i++)
			glgui::CLabel::PaintText(sCommands[i], sCommands[i].length(), L"sans-serif", 13, (float)(x + 5), (float)(y + h + 13 + iCommandsPainted++*13));

		if (bAbbreviated)
		{
			eastl::string16 sDotDotDot = L"...";
			glgui::CLabel::PaintText(sDotDotDot, sDotDotDot.length(), L"sans-serif", 13, (float)(x + 5), (float)(y + h + 13 + iCommandsPainted++*13));
		}
	}
}

void CConsole::PrintConsole(eastl::string16 sText)
{
	DebugPrint(sText);
	m_pOutput->AppendText(sText);

	if (!CApplication::Get()->IsOpen())
		return;

	if (IsVisible())
		Layout();
}

bool CConsole::KeyPressed(int code, bool bCtrlDown)
{
	if (!IsOpen())
		return false;

	if (code == TINKER_KEY_ESCAPE)
	{
		CApplication::Get()->CloseConsole();
		return true;
	}

	if (code == TINKER_KEY_ENTER || code == TINKER_KEY_KP_ENTER)
	{
		eastl::string16 sText = m_pInput->GetText();
		m_pInput->SetText("");

		PrintConsole(eastl::string16(L"] ") + sText + L"\n");

		CCommand::Run(sText);

		return true;
	}

	if (code == TINKER_KEY_TAB)
	{
		eastl::string16 sInput = m_pInput->GetText();
		if (sInput.length() && sInput.find(L' ') == ~0)
		{
			eastl::vector<eastl::string16> aCommands = CCommand::GetCommandsBeginningWith(sInput);
			if (aCommands.size())
			{
				m_pInput->SetText(aCommands[0] + L" ");
				m_pInput->SetCursorPosition(-1);
			}
		}
	}

	bool bReturn = BaseClass::KeyPressed(code, bCtrlDown);

	if (bReturn)
		return true;

	return false;
}

bool CConsole::CharPressed(int iKey)
{
	if (!IsOpen())
		return false;

	if (iKey == '`')
	{
		CApplication::Get()->CloseConsole();
		return true;
	}

	return BaseClass::CharPressed(iKey);
}

void CApplication::OpenConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	pConsole->Layout();
	pConsole->SetVisible(true);

	glgui::CRootPanel::Get()->MoveToTop(pConsole);
}

void CApplication::CloseConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	pConsole->SetVisible(false);
}

void CApplication::ToggleConsole()
{
	if (!Get())
		return;

	CConsole* pConsole = Get()->GetConsole();
	if (IsConsoleOpen())
		CloseConsole();
	else
		OpenConsole();
}

bool CApplication::IsConsoleOpen()
{
	if (!Get())
		return false;

	CConsole* pConsole = Get()->GetConsole();
	return pConsole->IsOpen();
}

void CApplication::PrintConsole(eastl::string16 sText)
{
	if (!Get())
	{
		puts(convertstring<char16_t, char>(sText).c_str());
		return;
	}

	CConsole* pConsole = Get()->GetConsole();
	pConsole->PrintConsole(sText);
}

void CApplication::PrintConsole(eastl::string sText)
{
	PrintConsole(convertstring<char, char16_t>(sText));
}

CConsole* CApplication::GetConsole()
{
	if (m_pConsole == NULL)
	{
		m_pConsole = new CConsole();
		m_pConsole->SetVisible(false);

		if (developer.GetBool())
			TMsg(L"Developer mode ON.\n");
	}

	return m_pConsole;
}
