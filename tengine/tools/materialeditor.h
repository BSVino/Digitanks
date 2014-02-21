#pragma once

#include <tstring.h>

#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/cameramanager.h>
#include <textures/materialhandle.h>
#include <tools/manipulator/manipulator.h>

#include "tool.h"

class CCreateMaterialPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateMaterialPanel, glgui::CMovablePanel);

public:
							CCreateMaterialPanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateMaterialPanel, MaterialChanged);
	EVENT_CALLBACK(CCreateMaterialPanel, ChooseShader);
	EVENT_CALLBACK(CCreateMaterialPanel, Create);

	tstring					GetMaterialFileName();

	void					FileNameChanged();

public:
	glgui::CLabel*			m_pMaterialFileLabel;
	glgui::CTextField*		m_pMaterialFileText;

	glgui::CMenu*			m_pShader;
	bool					m_bShaderChosen;

	glgui::CLabel*			m_pWarning;

	glgui::CButton*			m_pCreate;
};

class CMaterialPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMaterialPanel, glgui::CPanel);

public:
							CMaterialPanel();

public:
	void					Layout();

	EVENT_CALLBACK(CMaterialPanel, ChooseShader);
	EVENT_CALLBACK(CMaterialPanel, TextureParameterChanged);
	EVENT_CALLBACK(CMaterialPanel, ParameterChanged);
	EVENT_CALLBACK(CMaterialPanel, Save);

public:
	glgui::CLabel*			m_pName;

	glgui::CMenu*			m_pShader;

	glgui::CButton*			m_pSave;

	glgui::CPanel*			m_pParameterPanel;

	tvector<glgui::CLabel*>		m_apParameterLabels;
	tvector<glgui::CTextField*>	m_apParameterOptions;
	tvector<tstring>			m_asParameterNames;
};

class CMaterialEditor : public CWorkbenchTool
{
	DECLARE_CLASS(CMaterialEditor, CWorkbenchTool);

public:
							CMaterialEditor();
	virtual					~CMaterialEditor();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	virtual void			RenderScene();

	EVENT_CALLBACK(CMaterialEditor, NewMaterial);
	EVENT_CALLBACK(CMaterialEditor, SaveMaterial);
	EVENT_CALLBACK(CMaterialEditor, ChooseMaterial);
	EVENT_CALLBACK(CMaterialEditor, OpenMaterial);

	void					ResetPreviewDistance();

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, tinker_mouse_state_t iState);
	void					MouseMotion(int x, int y);
	void					MouseWheel(int x, int y);

	virtual TVector			GetCameraPosition();
	virtual Vector          GetCameraDirection();

	virtual tstring			GetToolName() { return "Material Editor"; }

	CMaterialHandle			GetMaterialPreview() { return m_hMaterial; }
	void					SetMaterial(CMaterialHandle hMaterial);

	static CMaterialEditor*	Get() { return s_pMaterialEditor; }

protected:
	CCreateMaterialPanel*	m_pCreateMaterialPanel;

	CMaterialHandle			m_hMaterial;

	CMaterialPanel*			m_pMaterialPanel;

	bool					m_bRotatingPreview;
	EAngle					m_angPreview;
	float					m_flPreviewDistance;

private:
	static CMaterialEditor*	s_pMaterialEditor;
};

inline CMaterialEditor* MaterialEditor()
{
	return CMaterialEditor::Get();
}
