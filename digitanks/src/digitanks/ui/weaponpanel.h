#ifndef DT_WEAPONPANEL_H
#define DT_WEAPONPANEL_H

#include <glgui/glgui.h>

#include <dt_common.h>

class CWeaponButton : public glgui::CPictureButton, public glgui::IEventListener
{
public:
									CWeaponButton(class CWeaponPanel* pPanel);

public:
	void							SetWeapon(weapon_t eWeapon);

	virtual void					CursorIn();

	EVENT_CALLBACK(CWeaponButton, ChooseWeapon);

public:
	class CWeaponPanel*				m_pWeaponPanel;

	weapon_t						m_eWeapon;
};

class CWeaponPanel : public glgui::CPanel, public glgui::IEventListener
{
public:
									CWeaponPanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	void							UpdateInfo(weapon_t eWeapon);

	void							SetTextureForWeapon(CWeaponButton* pWeapon, weapon_t eWeapon);

protected:
	glgui::CLabel*					m_pInfo;

	tvector<CWeaponButton*>			m_apWeapons;

	weapon_t						m_eWeapon;
};

#endif
