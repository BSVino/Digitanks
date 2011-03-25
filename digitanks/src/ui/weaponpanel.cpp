#include "weaponpanel.h"

#include <game/digitanks/digitanksgame.h>
#include <renderer/renderer.h>
#include "digitankswindow.h"
#include "hud.h"
#include "instructor.h"

using namespace glgui;

CWeaponPanel::CWeaponPanel()
	: CPanel(0, 0, 260, 300)
{
	m_pInfo = new CLabel(0, 0, 100, 300, L"");
	m_pInfo->SetFont(L"text");
	AddControl(m_pInfo);
}

void CWeaponPanel::Layout()
{
	CPanel::Layout();

	m_pInfo->SetSize(200, 250);
	m_pInfo->SetPos(GetWidth()+20, GetHeight()/2 - m_pInfo->GetHeight()/2);

	SetSize(250, 250);

	for (size_t i = 0; i < m_apWeapons.size(); i++)
	{
		m_apWeapons[i]->Destructor();
		m_apWeapons[i]->Delete();
	}

	m_apWeapons.clear();

	if (!DigitanksGame())
		return;

	CDigitanksTeam* pCurrentTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentTeam)
		return;

	CDigitank* pTank = pCurrentTeam->GetPrimarySelectionTank();
	if (!pTank)
		return;

	size_t iWeapons = pTank->GetNumWeapons();
	size_t iRows = (size_t)sqrt((float)iWeapons);

	int iButtonSize = GetWidth()/(iRows+1);

	if (iButtonSize > 64)
		iButtonSize = 64;

	int iXWidth = iButtonSize*20;
	int iYWidth = iButtonSize*20;

	int iPaddingSize = 10;

	int iMaxHeight = 0;

	size_t iWeapon = 0;
	for (size_t i = 0; i <= iRows; i++)
	{
		if (iWeapon >= pTank->GetNumWeapons())
			break;

		for (size_t j = 0; j <= iRows; j++)
		{
			if (iWeapon >= pTank->GetNumWeapons())
				break;

			weapon_t eWeapon = pTank->GetWeapon(iWeapon++);

			m_apWeapons.push_back(new CWeaponButton(this));
			CWeaponButton* pWeapon = m_apWeapons[m_apWeapons.size()-1];
			pWeapon->SetSize(iButtonSize-2, iButtonSize-1);
			pWeapon->SetPos(i*iButtonSize + iPaddingSize*i, j*iButtonSize + iPaddingSize*j);
			pWeapon->SetFont(L"text", 10);
			pWeapon->SetWeapon(eWeapon);
			AddControl(pWeapon);

			pWeapon->SetEnabled(true);
			pWeapon->SetFGColor(Color(0, 0, 0));
			pWeapon->SetClickedListener(pWeapon, &CWeaponButton::ChooseWeapon);

			SetTextureForWeapon(pWeapon, eWeapon);
			pWeapon->SetText(CProjectile::GetWeaponName(eWeapon));

			pWeapon->SetButtonColor(Color(0, 0, 0));

			int iHeight = (j+1)*iButtonSize + iPaddingSize*j;
			if (iHeight > iMaxHeight)
				iMaxHeight = iHeight;
		}
	}

	SetSize(iMaxHeight, iMaxHeight);

	SetPos(CRootPanel::Get()->GetWidth()/2 + 30, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	UpdateInfo(WEAPON_NONE);
}

void CWeaponPanel::Paint(int x, int y, int w, int h)
{
	if (DigitanksGame()->GetPrimarySelectionTank())
	{
		int iTankSize = 100;
		int iWindowWidth = DigitanksWindow()->GetWindowWidth();
		int iWindowHeight = DigitanksWindow()->GetWindowHeight();

		int iBoxSize = 250;
		glgui::CBaseControl::PaintRect(iWindowWidth/4 - iBoxSize/2, iWindowHeight/2 - iBoxSize/2, iBoxSize, iBoxSize, Color(0, 0, 0, 200));

		Color clrTeam(255, 255, 255, 255);
		if (DigitanksGame()->GetPrimarySelectionTank()->GetTeam())
			clrTeam = DigitanksGame()->GetPrimarySelectionTank()->GetTeam()->GetColor();

		CRenderingContext c(GameServer()->GetRenderer());

		c.SetBlend(BLEND_ALPHA);
		DigitanksWindow()->GetHUD()->PaintUnitSheet(DigitanksGame()->GetPrimarySelectionTank()->GetUnitType(), iWindowWidth/4 - iTankSize/2, iWindowHeight/2 - iTankSize/2, iTankSize, iTankSize, clrTeam);

		int iShieldSize = 200;

		int iShield = (int)(255*(10-CBaseWeapon::GetWeaponEnergy(m_eWeapon))/10);
		if (iShield > 255)
			iShield = 255;

		c.SetBlend(BLEND_ADDITIVE);
		glgui::CBaseControl::PaintTexture(DigitanksWindow()->GetHUD()->GetShieldTexture(), iWindowWidth/4 - iShieldSize/2, iWindowHeight/2 - iShieldSize/2, iShieldSize, iShieldSize, Color(255, 255, 255, iShield));

		c.SetColor(Color(255, 255, 255, 255));
		eastl::string16 sShields = sprintf(L"Shield Power: %d%%", 100-((int)CProjectile::GetWeaponEnergy(m_eWeapon)*10));
		float flTextWidth = glgui::CLabel::GetTextWidth(sShields, sShields.length(), L"text", 12);
		glgui::CLabel::PaintText(sShields, sShields.length(), L"text", 12, iWindowWidth/4 - flTextWidth/2, (float)(iWindowHeight/2 - iShieldSize/2));
	}

	if (m_pInfo->GetText().length() > 1)
	{
		int ix, iy, iw, ih;
		m_pInfo->GetAbsDimensions(ix, iy, iw, ih);
		CRootPanel::PaintRect(ix, iy, iw, ih, Color(0, 0, 0, GetAlpha()));
	}

	CPanel::Paint(x, y, w, h);
}

void CWeaponPanel::UpdateInfo(weapon_t eWeapon)
{
	m_eWeapon = eWeapon;

	if (!eWeapon)
	{
		m_pInfo->SetText("");
		return;
	}

	eastl::string16 s;
	eastl::string16 p;

	eastl::string16 sName = CProjectile::GetWeaponName(eWeapon);
	sName.make_upper();

	s += sName + L"\n \n";
	s += eastl::string16(CProjectile::GetWeaponDescription(eWeapon)) + L"\n \n";
	s += p.sprintf(L"Energy Required: %d%%\n", ((int)CProjectile::GetWeaponEnergy(eWeapon)*10));
	s += p.sprintf(L"Damage: %.1f\n", CProjectile::GetWeaponDamage(eWeapon));

	m_pInfo->SetText(s);

	m_pInfo->SetSize(m_pInfo->GetWidth(), 9999);
	m_pInfo->SetSize(m_pInfo->GetWidth(), (int)m_pInfo->GetTextHeight()+20);
}

void CWeaponPanel::SetTextureForWeapon(CWeaponButton* pWeapon, weapon_t eWeapon)
{
	int iTextureWidth = 512;
	int iTextureHeight = 256;

	int sx, sy, sw, sh, tw, th;
	CHUD::GetWeaponSheet(eWeapon, sx, sy, sw, sh, tw, th);

	pWeapon->SetSheetTexture(CHUD::GetWeaponSheet(), sx, sy, sw, sh, tw, th);
}

CWeaponButton::CWeaponButton(CWeaponPanel* pPanel)
	: CPictureButton(L"")
{
	m_pWeaponPanel = pPanel;
	m_eWeapon = WEAPON_NONE;
}

void CWeaponButton::SetWeapon(weapon_t eWeapon)
{
	m_eWeapon = eWeapon;
}

void CWeaponButton::CursorIn()
{
	CPictureButton::CursorIn();

	m_pWeaponPanel->UpdateInfo(m_eWeapon);
}

void CWeaponButton::ChooseWeaponCallback()
{
	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	for (size_t i = 0; i < pTeam->GetNumTanks(); i++)
	{
		CDigitank* pTank = pTeam->GetTank(i);
		if (!pTank || !pTeam->IsSelected(pTank))
			continue;

		pTank->SetCurrentWeapon(m_eWeapon);
	}

	m_pWeaponPanel->SetVisible(false);

	DigitanksGame()->SetControlMode(MODE_AIM);
	DigitanksGame()->SetAimTypeByWeapon(m_eWeapon);

	CRootPanel::Get()->Layout();

	DigitanksWindow()->GetInstructor()->FinishedTutorial("artillery-chooseweapon", true);
}

