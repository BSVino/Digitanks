#ifndef DT_UPDATESPANEL_H
#define DT_UPDATESPANEL_H

#include "glgui/glgui.h"

class CUpdateButton : public glgui::CPictureButton, public glgui::IEventListener
{
public:
									CUpdateButton(class CUpdatesPanel* pPanel);

public:
	void							SetLocation(int x, int y);

	virtual void					CursorIn();
	virtual void					CursorOut();

	EVENT_CALLBACK(CUpdateButton, ChooseDownload);

public:
	class CUpdatesPanel*			m_pUpdatesPanel;

	int								m_iX;
	int								m_iY;
};

class CUpdatesPanel : public glgui::CPanel, public glgui::IEventListener
{
public:
									CUpdatesPanel();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	EVENT_CALLBACK(CUpdatesPanel, Close);

	void							UpdateInfo(class CUpdateItem* pInfo);

	void							GetTextureForUpdateItem(class CUpdateItem* pInfo, size_t& iSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th);

	int								GetButtonSize() { return m_iButtonSize; }

protected:
	glgui::CButton*					m_pCloseButton;
	glgui::CLabel*					m_pAvailable;
	glgui::CLabel*					m_pInfo;
	glgui::CLabel*					m_pTutorial;

	eastl::vector<CUpdateButton*>	m_apUpdates;

	int								m_iButtonSize;
};

#endif
