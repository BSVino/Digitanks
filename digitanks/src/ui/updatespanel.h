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

	size_t							GetTextureForUpdateItem(class CUpdateItem* pInfo);

protected:
	glgui::CButton*					m_pCloseButton;
	glgui::CLabel*					m_pInfo;

	eastl::vector<CUpdateButton*>	m_apUpdates;

	size_t							m_iIconCPU;
	size_t							m_iIconBuffer;
	size_t							m_iIconPSU;
	size_t							m_iIconInfantryLoader;
	size_t							m_iIconTankLoader;
	size_t							m_iIconArtilleryLoader;
	size_t							m_iIconInfantryAttack;
	size_t							m_iIconInfantryDefense;
	size_t							m_iIconInfantryMovement;
	size_t							m_iIconInfantryHealth;
	size_t							m_iIconTankAttack;
	size_t							m_iIconTankDefense;
	size_t							m_iIconTankMovement;
	size_t							m_iIconTankHealth;
	size_t							m_iIconArtilleryAttack;
	size_t							m_iIconArtilleryRange;
	size_t							m_iIconArtilleryMovement;
	size_t							m_iIconArtilleryHealth;
	size_t							m_iIconBufferRecharge;
	size_t							m_iIconBufferEnergy;
	size_t							m_iIconBufferFleet;
	size_t							m_iIconBufferBandwidth;
	size_t							m_iIconCPUPower;
	size_t							m_iIconCPUFleet;
	size_t							m_iIconCPUBandwidth;
};

#endif
