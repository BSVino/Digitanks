/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "textfield.h"

#include <FTGL/ftgl.h>

#include <tinker_platform.h>
#include <files.h>

#include <tinker/keys.h>
#include <tinker/application.h>
#include <renderer/shaders.h>
#include <renderer/renderingcontext.h>

#include "label.h"
#include "rootpanel.h"

using namespace glgui;

typedef char FTGLchar;

CTextField::CTextField()
	: CBaseControl(0, 0, 140, 30)
{
	m_bEnabled = true;
	m_FGColor = Color(255, 255, 255, 255);

	SetFontFaceSize(13);

	SetSize(GetWidth(), GetTextHeight() + 8.0f);

	m_iCursor = 0;
	m_iSelection = m_iCursor;

	m_flBlinkTime = 0;

	m_flRenderOffset = 0;

	m_pfnContentsChangedCallback = NULL;
	m_pContentsChangedListener = NULL;

	m_iAutoComplete = -1;
}

void CTextField::Paint(float x, float y, float w, float h)
{
	if (!IsVisible())
		return;

	if (m_iAlpha == 0)
		return;

	if (IsEnabled())
		glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 50), 3);
	else
	{
		glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, 50), 3);
		glgui::CRootPanel::PaintRect(x, y, w, h, Color(100, 100, 100, 50), 0);
	}

	if (HasFocus())
	{
		float flCursor = CLabel::GetTextWidth(m_sText, m_iCursor, CLabel::GetFont("sans-serif", m_iFontFaceSize));

		if (m_iCursor != m_iSelection)
		{
			float flSelection = CLabel::GetTextWidth(m_sText, m_iSelection, CLabel::GetFont("sans-serif", m_iFontFaceSize));
			float flStart = std::min(flCursor, flSelection);
			float flEnd = std::max(flCursor, flSelection);
			glgui::CRootPanel::PaintRect(x + 4 + flStart + m_flRenderOffset, y+3, flEnd-flStart, h-6, Color(50, 50, 200, 155), 1);
		}

		if (fmod(CRootPanel::Get()->GetTime() - m_flBlinkTime, 1) < 0.5f)
			glgui::CRootPanel::PaintRect(x + 4 + flCursor + m_flRenderOffset, y+3, 1, h-6, Color(200, 200, 200, 255), 1);
	}

	Color clrFG = GetFGColor();

	CRootPanel::GetContext()->SetUniform("vecColor", clrFG);

	FTFont* pFont = CLabel::GetFont("sans-serif", m_iFontFaceSize);

	DrawLine(m_sText.c_str(), (unsigned int)m_sText.length(), x+4, y, w-8, h, clrFG);

	if (m_sText.length() && m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size() && HasFocus())
	{
		int iAutoComplete = m_iAutoComplete % m_asAutoCompleteCommands.size();
		tstring sCommand = m_asAutoCompleteCommands[iAutoComplete];

		if (m_bSlashInsensitive)
			TAssertNoMsg(ToForwardSlashes(sCommand).compare(0, m_sText.length(), ToForwardSlashes(m_sText)) == 0)	// By an amazing oddity of preproccesor magic the semicolon is not welcome here.
		else
			TAssertNoMsg(sCommand.compare(0, m_sText.length(), m_sText) == 0);

		sCommand = sCommand.substr(m_sText.length());

		float flOriginalTextWidth = CLabel::GetTextWidth(m_sText, m_sText.length(), "sans-serif", m_iFontFaceSize);
		DrawLine(sCommand.c_str(), sCommand.length(), x+4+flOriginalTextWidth, y, w-8-flOriginalTextWidth, h, clrFG/2);
	}
}

void CTextField::PostPaint()
{
	tstring sInput = GetText();
	if (sInput.length() && m_asAutoCompleteCommands.size() && HasFocus())
	{
		float x, y, w, h;
		GetAbsDimensions(x, y, w, h);

		size_t iCommandsToShow = m_asAutoCompleteCommands.size();
		bool bAbbreviated = false;

		if (iCommandsToShow > 5)
		{
			iCommandsToShow = 5;
			bAbbreviated = true;
		}

		if (bAbbreviated)
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, (float)(iCommandsToShow+1)*13+3, Color(0, 0, 0, 200), 0, true);
		else
			glgui::CRootPanel::PaintRect(x+5, y+h+2, w, (float)iCommandsToShow*13+3, Color(0, 0, 0, 200), 0, true);

		int iCommandsToSkip = 0;
		if (m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size())
		{
			int iAutoComplete = m_iAutoComplete % m_asAutoCompleteCommands.size();

			if (iAutoComplete < 5)
				glgui::CRootPanel::PaintRect(x+5, y+h+2 + 13*iAutoComplete, w, 13+3, Color(100, 100, 100, 200), 2);
			else
			{
				glgui::CRootPanel::PaintRect(x+5, y+h+2 + 13*4, w, 13+3, Color(100, 100, 100, 200), 2);
				iCommandsToSkip = iAutoComplete - 4;
			}

			if (iAutoComplete == m_asAutoCompleteCommands.size()-1)
				bAbbreviated = false;
		}

		int iCommandsPainted = 0;
		for (size_t i = iCommandsToSkip; i < iCommandsToShow+iCommandsToSkip; i++)
			glgui::CLabel::PaintText(m_asAutoCompleteCommands[i], m_asAutoCompleteCommands[i].length(), "sans-serif", 13, (float)(x + 5), (float)(y + h + iCommandsPainted++*13));

		if (bAbbreviated)
		{
			tstring sDotDotDot = "...";
			glgui::CLabel::PaintText(sDotDotDot, sDotDotDot.length(), "sans-serif", 13, (float)(x + 5), (float)(y + h + iCommandsPainted++*13));
		}
	}
}

void CTextField::DrawLine(const tchar* pszText, unsigned iLength, float x, float y, float w, float h, const Color& clrLine)
{
	if (!iLength)
		return;

	FTFont* pFont = CLabel::GetFont("sans-serif", m_iFontFaceSize);

	float flMargin = (h - CLabel::GetFontHeight(pFont)) / 2;
	Vector vecPosition = Vector((float)x + m_flRenderOffset, (float)CRootPanel::Get()->GetBottom() - y + flMargin - CLabel::GetFontHeight(pFont), 0);

	// Intentionally using the viewport size instead of the root panel size to render.
	// We scaled the font size requested by the gui scale factor and now ftgl can render
	// it as if it were regular sized, meaning it will look larger.
	size_t vw, vh;
	Application()->GetViewportSize(vw, vh);
	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, (float)vw, 0, (float)vh, -1, 1);

	float cx, cy;
	GetAbsPos(cx, cy);

	FRect r(-1, -1, -1, -1);
	CPanel* pParent = GetParent().Downcast<CPanel>();
	while (pParent)
	{
		if (pParent && pParent->IsScissoring())
		{
			pParent->GetAbsPos(r.x, r.y);
			r.w = pParent->GetWidth();
			r.h = pParent->GetHeight();
			break;
		}
		pParent = pParent->GetParent().Downcast<CPanel>();
	}

	FRect r2;
	GetAbsPos(r2.x, r2.y);
	r2.w = GetWidth();
	r2.h = GetHeight();

	r2.x += 4;
	r2.y += 4;
	r2.w -= 8;
	r2.h -= 4;

	if (r.x < 0)
	{
		r = r2;
	}
	else
	{
		if (!r.Union(r2))
			r = FRect(-1, -1, -1, -1);
	}

	Vector4D vecStencil(&r.x);
	vecStencil.y = CRootPanel::Get()->GetBottom()-vecStencil.y-vecStencil.w;

	::CRenderingContext c(nullptr, true);
	c.SetBlend(BLEND_ALPHA);
	c.UseProgram("text");
	c.SetUniform("bScissor", true);
	c.SetUniform("vecScissor", vecStencil / Application()->GetGUIScale());
	c.SetUniform("vecColor", m_FGColor);
	c.SetProjection(mFontProjection);
	c.Translate(vecPosition / Application()->GetGUIScale());

	c.RenderText(pszText, iLength, "sans-serif", m_iFontFaceSize);
}

bool CTextField::TakesFocus()
{
	return IsVisible();
}

bool CTextField::SetFocus(bool bFocus)
{
	bool bResult = CBaseControl::SetFocus(bFocus);

	if (!bResult)
		return false;

	m_iAutoComplete = -1;

	if (bFocus)
	{
		m_flBlinkTime = CRootPanel::Get()->GetTime();

		m_iSelection = m_iCursor = m_sText.length();

		FRect r = GetAbsDimensions();
		r /= Application()->GetGUIScale();
		Application()->ActivateKeyboard(r);

		return true;
	}

	Application()->DeactivateKeyboard();

	return HasFocus();
}

bool CTextField::MousePressed(int iButton, int mx, int my)
{
	if (!TakesFocus())
		return false;

	if (!m_bEnabled)
		return false;

	CRootPanel::Get()->SetFocus(m_hThis);

	float cx, cy;
	GetAbsPos(cx, cy);

	float flCursor = (float)(mx-cx);
	for (size_t i = 1; i < m_sText.length(); i++)
	{
		float flText = CLabel::GetTextWidth(m_sText, i, CLabel::GetFont("sans-serif", m_iFontFaceSize));
		if (flCursor < flText)
		{
			m_iCursor = i-1;

			if (!Application()->IsShiftDown())
				m_iSelection = m_iCursor;

			return true;
		}
	}

	m_iCursor = m_sText.length();

	if (!Application()->IsShiftDown())
		m_iSelection = m_iCursor;

	return true;
}

void CTextField::CursorMoved(int x, int y, int dx, int dy)
{
	if (!Application()->IsMouseLeftDown())
		return;

	if (!TakesFocus())
		return;

	if (!HasFocus())
		return;

	float cx, cy;
	GetAbsPos(cx, cy);

	float flCursor = (float)(x-cx);
	for (size_t i = 1; i < m_sText.length(); i++)
	{
		float flText = CLabel::GetTextWidth(m_sText, i, CLabel::GetFont("sans-serif", m_iFontFaceSize));
		if (flCursor < flText)
		{
			m_iCursor = i-1;

			return;
		}
	}

	m_iCursor = m_sText.length();
}

bool CTextField::CharPressed(int iKey)
{
	if (HasFocus())
	{
		if (m_iCursor != m_iSelection)
		{
			size_t iStart = std::min(m_iCursor, m_iSelection);
			size_t iEnd = std::max(m_iCursor, m_iSelection);
			m_sText.erase(iStart, iEnd-iStart);
			m_iCursor = m_iSelection = iStart;
		}

		if (iKey <= TINKER_KEY_FIRST)
		{
			m_sText.insert(m_iCursor++, 1, iKey);
			UpdateContentsChangedListener();
		}

		m_flBlinkTime = CRootPanel::Get()->GetTime();

		FindRenderOffset();

		m_iSelection = m_iCursor;

		return true;
	}

	return CBaseControl::CharPressed(iKey);
}

bool CTextField::KeyPressed(int iKey, bool bCtrlDown)
{
	if (m_sText.length() && m_iAutoComplete >= 0 && m_asAutoCompleteCommands.size())
	{
		if (iKey == TINKER_KEY_TAB || iKey == TINKER_KEY_DOWN)
		{
			m_iAutoComplete++;
		}
		else if (iKey == TINKER_KEY_UP)
		{
			m_iAutoComplete--;
		}
		else if (iKey == TINKER_KEY_BACKSPACE || iKey == TINKER_KEY_LEFT || iKey == TINKER_KEY_RIGHT || iKey == TINKER_KEY_DEL || iKey == TINKER_KEY_HOME || iKey == TINKER_KEY_END)
		{
			// Let the text field handle it.
			m_iAutoComplete = -1;
		}
		else
		{
			tstring sInput = GetText();
			if (sInput.length())
			{
				SetText(m_asAutoCompleteCommands[m_iAutoComplete % m_asAutoCompleteCommands.size()]);
				SetCursorPosition(-1);
				UpdateContentsChangedListener();
			}

			m_iAutoComplete = -1;
		}

		return true;
	}
	else if (iKey == TINKER_KEY_ESCAPE || iKey == TINKER_KEY_ENTER)
		CRootPanel::Get()->SetFocus(CControlHandle());
	else if (iKey == TINKER_KEY_LEFT)
	{
		if (bCtrlDown)
		{
			if (m_iCursor)
			{
				m_iCursor -= 2;
				while (m_iCursor > 0 && m_sText[m_iCursor] != ' ')
					m_iCursor--;
				if (m_sText[m_iCursor] == ' ')
					m_iCursor++;
			}
		}
		else if (m_iCursor > 0)
			m_iCursor--;

		if (!Application()->IsShiftDown())
			m_iSelection = m_iCursor;
	}
	else if (iKey == TINKER_KEY_RIGHT)
	{
		if (bCtrlDown)
		{
			m_iCursor++;
			while (m_iCursor < m_sText.length() && m_sText[m_iCursor] != ' ')
				m_iCursor++;
		}
		else if (m_iCursor < m_sText.length())
			m_iCursor++;

		if (!Application()->IsShiftDown())
			m_iSelection = m_iCursor;
	}
	else if (iKey == TINKER_KEY_BACKSPACE)
	{
		if (m_iCursor != m_iSelection)
		{
			size_t iStart = std::min(m_iCursor, m_iSelection);
			size_t iEnd = std::max(m_iCursor, m_iSelection);
			m_sText.erase(iStart, iEnd-iStart);
			m_iCursor = m_iSelection = iStart;
			UpdateContentsChangedListener();
		}
		else if (m_iCursor > 0)
		{
			m_sText.erase(m_iCursor-1, 1);
			m_iCursor--;
			m_iSelection = m_iCursor;
			UpdateContentsChangedListener();
		}
	}
	else if (iKey == TINKER_KEY_DEL)
	{
		if (m_iCursor != m_iSelection)
		{
			size_t iStart = std::min(m_iCursor, m_iSelection);
			size_t iEnd = std::max(m_iCursor, m_iSelection);
			m_sText.erase(iStart, iEnd-iStart);
			m_iCursor = m_iSelection = iStart;
		}
		else if (m_iCursor < m_sText.length())
		{
			m_sText.erase(m_iCursor, 1);
			UpdateContentsChangedListener();
		}
	}
	else if (iKey == TINKER_KEY_HOME)
	{
		m_iCursor = 0;

		if (!Application()->IsShiftDown())
			m_iSelection = m_iCursor;
	}
	else if (iKey == TINKER_KEY_END)
	{
		m_iCursor = m_sText.length();

		if (!Application()->IsShiftDown())
			m_iSelection = m_iCursor;
	}
	else if ((iKey == 'v' || iKey == 'V' || iKey == '.') && bCtrlDown)	// '.' because dvorak screws with my hotkeys
	{
		if (m_iCursor != m_iSelection)
		{
			size_t iStart = std::min(m_iCursor, m_iSelection);
			size_t iEnd = std::max(m_iCursor, m_iSelection);
			m_sText.erase(iStart, iEnd-iStart);
			m_iCursor = m_iSelection = iStart;
		}

		tstring sClipboard = GetClipboard();
		m_sText.insert(m_sText.begin()+m_iCursor, sClipboard.begin(), sClipboard.end());
		m_iSelection = m_iCursor = m_iCursor + sClipboard.length();
		UpdateContentsChangedListener();
	}
	else if ((iKey == 'a' || iKey == 'A') && bCtrlDown)
	{
		m_iSelection = 0;
		m_iCursor = m_sText.length();

		UpdateContentsChangedListener();
	}
	else if (m_sText.length() && m_asAutoCompleteCommands.size() && (iKey == TINKER_KEY_TAB || iKey == TINKER_KEY_DOWN))
	{
		m_iAutoComplete++;
		return true;
	}
	else if (iKey == TINKER_KEY_UP)
	{
		m_iAutoComplete--;
		return true;
	}

	m_flBlinkTime = CRootPanel::Get()->GetTime();

	FindRenderOffset();

	return iKey != TINKER_KEY_TAB;	// Let the panel handle tab.
}

void CTextField::SetContentsChangedListener(IEventListener* pListener, IEventListener::Callback pfnCallback, const tstring& sArgs)
{
	m_pfnContentsChangedCallback = pfnCallback;
	m_pContentsChangedListener = pListener;
	m_sContentsChangedArgs = sArgs;
}

void CTextField::UpdateContentsChangedListener()
{
	if (m_pfnContentsChangedCallback)
		m_pfnContentsChangedCallback(m_pContentsChangedListener, m_sContentsChangedArgs);
}

void CTextField::FindRenderOffset()
{
	float cx, cy;
	GetAbsPos(cx, cy);

	float flTextWidth = CLabel::GetTextWidth(m_sText, m_sText.length(), CLabel::GetFont("sans-serif", m_iFontFaceSize));
	float flCursorOffset = CLabel::GetTextWidth(m_sText, m_iCursor, CLabel::GetFont("sans-serif", m_iFontFaceSize));

	float flTextLeft = (cx + 4) + m_flRenderOffset;
	float flTextRight = flTextLeft + flTextWidth + m_flRenderOffset;
	float flCursorPosition = flTextLeft + flCursorOffset;

	float flLeftOverrun = (cx + 4) - flCursorPosition;
	float flRightOverrun = flCursorPosition - (cx + GetWidth() - 4);

	if (flLeftOverrun > 0)
	{
		m_flRenderOffset += (flLeftOverrun+25);
		if (m_flRenderOffset > 0)
			m_flRenderOffset = 0;
	}
	else if (flRightOverrun > 0)
		m_flRenderOffset -= flRightOverrun;
}

void CTextField::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;

	if (!bEnabled && CRootPanel::Get()->GetFocus() == this)
		RootPanel()->SetFocus(CControlHandle());
}

void CTextField::SetText(const tstring& sText)
{
	bool bContentsChanged = (m_sText != sText);

	m_sText = sText;

	if (m_iCursor > m_sText.length())
		m_iCursor = m_sText.length();

	if (m_iSelection > m_sText.length())
		m_iSelection = m_sText.length();

	if (bContentsChanged)
		UpdateContentsChangedListener();
}

void CTextField::AppendText(const tchar* pszText)
{
	if (!pszText)
		return;

	m_sText.append(pszText);
}

void CTextField::ClearAutoCompleteCommands()
{
	m_asAutoCompleteCommands.clear();
}

void CTextField::SetAutoCompleteCommands(const tvector<tstring>& asCommands, bool bSlashInsensitive)
{
	m_bSlashInsensitive = bSlashInsensitive;

	m_asAutoCompleteCommands.clear();

	for (size_t i = 0; i < asCommands.size(); i++)
	{
		if (asCommands[i] == m_sText)
			continue;

		if (m_bSlashInsensitive)
		{
			if (ToForwardSlashes(asCommands[i]).compare(0, m_sText.length(), ToForwardSlashes(m_sText)) == 0)
				m_asAutoCompleteCommands.push_back(asCommands[i]);
		}
		else
		{
			if (asCommands[i].compare(0, m_sText.length(), m_sText) == 0)
				m_asAutoCompleteCommands.push_back(asCommands[i]);
		}
	}

	m_iAutoComplete = -1;
}

void CTextField::SetAutoCompleteFiles(const tstring& sBaseDirectory, const tvector<tstring>& asExtensions, const tvector<tstring>& asExtensionsExclude)
{
	tstring sSourceFolder = FindAbsolutePath(sBaseDirectory);
	tstring sInputFolder = FindAbsolutePath(sBaseDirectory + "/" + GetText());

	if (!sBaseDirectory.length())
	{
		sSourceFolder = FindAbsolutePath(GetDirectory(GetText()));
		sInputFolder = FindAbsolutePath(GetText());
	}

	if (sInputFolder.compare(0, sSourceFolder.length(), sSourceFolder) != 0)
		return;

	tstring sSearchDirectory = GetDirectory(sInputFolder);

	tstring sPrefix;
	if (sSourceFolder == sInputFolder)
		sPrefix = "";
	else if (!sBaseDirectory.length())
		sPrefix = ToForwardSlashes(sSourceFolder);
	else if (sSourceFolder.length() <= sSearchDirectory.length())
		sPrefix = ToForwardSlashes(sSearchDirectory.substr(sSourceFolder.length()));
	while (sPrefix[0] == '/')
		sPrefix = sPrefix.substr(1);
	while (sPrefix.back() == '/')
		sPrefix = sPrefix.substr(0, sPrefix.length()-2);
	if (sPrefix.length())
		sPrefix = sPrefix + '/';

	tvector<tstring> asFiles = ListDirectory(sSearchDirectory);
	tvector<tstring> asCompletions;

	for (size_t i = 0; i < asFiles.size(); i++)
	{
		if (!IsDirectory(sSearchDirectory + '/' + asFiles[i]))
		{
			tstring sFileName = asFiles[i].tolower();

			if (asExtensions.size())
			{
				bool bFound = false;
				for (size_t j = 0; j < asExtensions.size(); j++)
				{
					if (!sFileName.endswith(asExtensions[j]))
						continue;

					bFound = true;
					break;
				}

				if (!bFound)
					continue;
			}

			if (asExtensionsExclude.size())
			{
				bool bFound = false;
				for (size_t j = 0; j < asExtensionsExclude.size(); j++)
				{
					if (sFileName.length() <= asExtensionsExclude[j].length())
						continue;

					if (sFileName.substr(sFileName.length()-asExtensionsExclude[j].length()) != asExtensionsExclude[j])
						continue;

					bFound = true;
					break;
				}

				if (bFound)
					continue;
			}
		}

		asCompletions.push_back(sPrefix + asFiles[i]);
	}

	SetAutoCompleteCommands(asCompletions, true);
}

void CTextField::SetCursorPosition(size_t iPosition)
{
	m_iSelection = m_iCursor = iPosition;

	if (m_iCursor > m_sText.length())
		m_iCursor = m_sText.length();

	if (m_iSelection > m_sText.length())
		m_iSelection = m_sText.length();
}

void CTextField::SetFontFaceSize(int iSize)
{
	if (!CLabel::GetFont("sans-serif", iSize))
		CLabel::AddFontSize("sans-serif", iSize);

	m_iFontFaceSize = iSize;
}

float CTextField::GetTextWidth()
{
	return CLabel::GetTextWidth(m_sText, m_sText.length(), CLabel::GetFont("sans-serif", m_iFontFaceSize));
}

float CTextField::GetTextHeight()
{
	return CLabel::GetFontHeight(CLabel::GetFont("sans-serif", m_iFontFaceSize));
}

// Make the label tall enough for one line of text to fit inside.
void CTextField::EnsureTextFits()
{
	float w = GetTextWidth()+4;
	float h = GetTextHeight()+4;

	if (m_flH < h)
		SetSize(m_flW, h);

	if (m_flW < w)
		SetSize(w, m_flH);
}

tstring CTextField::GetText()
{
	return m_sText;
}

Color CTextField::GetFGColor()
{
	if (!m_bEnabled)
	{
		Color clrDisabled = m_FGColor;
		clrDisabled /= 2;
		return clrDisabled;
	}

	return m_FGColor;
}

void CTextField::SetFGColor(Color FGColor)
{
	m_FGColor = FGColor;
	SetAlpha(FGColor.a());
}

void CTextField::SetAlpha(int a)
{
	CBaseControl::SetAlpha(a);
	m_FGColor.SetAlpha(a);
}
