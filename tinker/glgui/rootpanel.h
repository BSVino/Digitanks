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

#ifndef TINKER_ROOTPANEL_H
#define TINKER_ROOTPANEL_H

#include "glgui.h"
#include "panel.h"

class CRenderingContext;
class FTFont;

namespace glgui
{
	class CRootPanel : public CPanel
	{
		DECLARE_CLASS(CRootPanel, CPanel);

	public:
									CRootPanel( );
		virtual						~CRootPanel( );

	public:
		virtual void SetDesignHeight(float flHeight); // Set to < 0 to disable.

		virtual void				Think(double flTime);
		virtual void				UpdateScene();
		virtual void				Paint(float x, float y, float w, float h);
		virtual void				Layout();

		virtual bool				MousePressed(int code, int mx, int my, bool bInsideControl = false);
		virtual bool				MouseReleased(int code, int mx, int my);
		virtual bool				MouseDoubleClicked(int code, int mx, int my);
		virtual void				CursorMoved(int mx, int my);

		// Dragon Drop stuff is in this class, because this is always the
		// top-level panel so all the messages go through it first.
		virtual void				DragonDrop(IDroppable* pDroppable);
		virtual void				AddDroppable(IDroppable* pDroppable);
		virtual void				RemoveDroppable(IDroppable* pDroppable);
		virtual bool				DropDraggable();
		virtual IDraggable*			GetCurrentDraggable() { return m_pDragging?m_pDragging->GetCurrentDraggable():NULL; };
		virtual IDroppable*			GetCurrentDroppable() { return m_pDragging; };

		bool						SetFocus(CControlHandle hControl);
		CControlHandle              GetFocus() const { return m_hFocus; }

		void						SetButtonDown(CControl<CButton> pButton);
		CControl<CButton>			GetButtonDown() const;

		CControl<CMenuBar>			GetMenuBar() { return m_hMenuBar; };
		CControl<CMenu>				AddMenu(const tstring& sText);

		double						GetFrameTime() { return m_flFrameTime; };
		double						GetTime() { return m_flTime; };

		void                        CollectGarbage();
		void                        CollectGarbageFinal();
		bool						IsGarbageCollecting() const { return m_bGarbageCollecting; }
		bool						IsDrawingDraggable() const { return m_bDrawingDraggable; }

		::FTFont* GetFont(const tstring& sName, size_t iSize);
		void      AddFont(const tstring& sName, const tstring& sFile);
		void      AddFontSize(const tstring& sName, size_t iSize);

		float GetTextWidth(const tstring& sText, unsigned iLength, const tstring& sFontName, int iFontFaceSize);
		float GetFontHeight(const tstring& sFontName, int iFontFaceSize);
		float GetFontAscender(const tstring& sFontName, int iFontFaceSize);

		float GetTextWidth(const tstring& sText, unsigned iLength, class ::FTFont* pFont);
		float GetFontHeight(class ::FTFont* pFont);
		float GetFontAscender(class ::FTFont* pFont);
		float GetFontDescender(class ::FTFont* pFont);

		void MakeQuad();
		size_t GetQuad();

		static CRootPanel*			Get();
		static bool                 Exists();
		static void                 Reset();

		static void					GetFullscreenMousePos(int& mx, int& my);

		static ::CRenderingContext*	GetContext() { return Get()->m_pRenderingContext; }

	private:
		static CControlResource	s_pRootPanel;
		static bool					s_bRootPanelValid;

		float m_flDesignHeight;

		tvector<IDroppable*>		m_apDroppables;
		IDroppable*					m_pDragging;

		// If the mouse is released over nothing, then try popping this button.
		CControl<CButton>			m_hButtonDown;

		CControlHandle				m_hFocus;

		CControl<CMenuBar>			m_hMenuBar;

		double						m_flFrameTime;
		double						m_flTime;

		double						m_flNextGCSweep;

		int							m_iMX;
		int							m_iMY;

		bool						m_bGarbageCollecting;
		bool						m_bDrawingDraggable;

		::CRenderingContext*		m_pRenderingContext;

		class CFont
		{
		public:
			tstring m_sFileName;
			tstring m_sFileContents;

			tmap<size_t, ::FTFont*> m_apFonts;
		};

		tmap<tstring, CFont> m_aFonts;

		// A quad for drawing GUI elements
		size_t m_iQuad;
	};

	inline CRootPanel* RootPanel()
	{
		return CRootPanel::Get();
	}
};

#endif
