#ifndef DT_GENERAL_WINDOW_H
#define DT_GENERAL_WINDOW_H

#include <glgui/glgui.h>
#include <models/texturesheet.h>

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
	virtual void		Paint(int x, int y, int w, int h);

	void				Reset();
	void				Deploy();
	void				RetryDebugging();
	void				GiveUpDebugging();
	void				DigitanksWon();

	EVENT_CALLBACK(CGeneralWindow, ButtonPressed);

public:
	float				m_flDeployedGoal;
	float				m_flDeployed;

	float				m_flStartTime;
	bool				m_bHelperSpeaking;

	glgui::CLabel*		m_pText;
	glgui::CButton*		m_pButton;

	CTextureSheet		m_hAntivirus;
	CTextureSheet		m_hGeneral;
	CTextureSheet		m_hGeneralMouth;
	eastl::string		m_sEmotion;

	generalstage_t		m_eStage;

	float				m_flFadeToBlack;
};

#endif
