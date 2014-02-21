#ifndef DT_GENERAL_WINDOW_H
#define DT_GENERAL_WINDOW_H

#include <glgui/glgui.h>
#include <glgui/panel.h>
#include <textures/texturesheet.h>

typedef enum
{
	STAGE_REPAIR,
	STAGE_REPAIR2,
	STAGE_DIGITANKS,
	STAGE_INTEL,
	STAGE_FINISH,
} generalstage_t;

class CGeneralWindow : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CGeneralWindow, glgui::CPanel);

public:
						CGeneralWindow();

public:
	virtual void		Layout();
	virtual void		Think();
	virtual void		Paint(float x, float y, float w, float h);

	void				Reset();
	void				Deploy();
	void				RetryDebugging();
	void				GiveUpDebugging();
	void				DigitanksWon();

	EVENT_CALLBACK(CGeneralWindow, ButtonPressed);

public:
	float				m_flDeployedGoal;
	float				m_flDeployed;

	double				m_flStartTime;
	bool				m_bHelperSpeaking;
	bool				m_bProgressBar;

	glgui::CControl<glgui::CLabel>		m_pText;
	glgui::CControl<glgui::CButton>		m_pButton;

	CTextureSheet		m_hAntivirus;
	CTextureSheet		m_hGeneral;
	CTextureSheet		m_hGeneralMouth;
	tstring				m_sEmotion;

	generalstage_t		m_eStage;

	double				m_flFadeToBlack;
};

#endif
