#ifndef DT_CHATBOX_H
#define DT_CHATBOX_H

#include <glgui/panel.h>

class CChatBox : public glgui::CPanel
{
	DECLARE_CLASS(CChatBox, glgui::CPanel);

public:
							CChatBox(bool bFloating = false);
	virtual					~CChatBox();

public:
	virtual bool			IsVisible();
	virtual void			SetVisible(bool bVisible);
	virtual bool			IsOpen();

	virtual void			Layout();
	virtual void			Paint(float x, float y, float w, float h);

	void					PrintChat(tstring sText);

	virtual bool			KeyPressed(int code, bool bCtrlDown = false);
	virtual bool			CharPressed(int iKey);

	virtual bool			IsFloating() { return m_bFloating; }

protected:
	glgui::CLabel*			m_pOutput;
	glgui::CTextField*		m_pInput;

	double					m_flLastMessage;

	bool					m_bFloating;
};

#endif
