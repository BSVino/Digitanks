#ifndef DT_CHATBOX_H
#define DT_CHATBOX_H

#include <glgui/glgui.h>

class CChatBox : public glgui::CPanel
{
	DECLARE_CLASS(CChatBox, glgui::CPanel);

public:
							CChatBox();
	virtual					~CChatBox();

public:
	virtual void			Delete() { delete this; };

	virtual bool			IsVisible();
	virtual void			SetVisible(bool bVisible);
	virtual bool			IsOpen();

	virtual void			Layout();
	virtual void			Paint(int x, int y, int w, int h);

	void					PrintChat(eastl::string16 sText);

	virtual bool			KeyPressed(int code, bool bCtrlDown = false);
	virtual bool			CharPressed(int iKey);

protected:
	glgui::CLabel*			m_pOutput;
	glgui::CTextField*		m_pInput;

	float					m_flLastMessage;
};

#endif
