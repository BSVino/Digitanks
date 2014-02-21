#pragma once

#include "tool.h"

#include <glgui/panel.h>

class CToyPreviewPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CToyPreviewPanel, glgui::CPanel);

public:
							CToyPreviewPanel();

public:
	void					Layout();

public:
	glgui::CLabel*			m_pInfo;

	glgui::CLabel*			m_pShowPhysicsLabel;
	glgui::CCheckBox*		m_pShowPhysics;
};

class CToyViewer : public CWorkbenchTool
{
	DECLARE_CLASS(CToyViewer, CWorkbenchTool);

public:
							CToyViewer();
	virtual					~CToyViewer();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	void					Layout();
	void					SetupMenu();

	virtual void			RenderScene();

	EVENT_CALLBACK(CToyViewer, ChooseToy);
	EVENT_CALLBACK(CToyViewer, OpenToy);

	bool					MouseInput(int iButton, tinker_mouse_state_t iState);
	void					MouseMotion(int x, int y);
	void					MouseWheel(int x, int y);

	virtual TVector			GetCameraPosition();
	virtual Vector          GetCameraDirection();

	virtual tstring			GetToolName() { return "Toy Viewer"; }

	virtual tstring			GetToyPreview() { return m_sToyPreview; }

public:
	static CToyViewer*		Get() { return s_pToyViewer; }

protected:
	CToyPreviewPanel*		m_pToyPreviewPanel;

	tstring					m_sToyPreview;
	size_t					m_iToyPreview;

	bool					m_bRotatingPreview;
	EAngle					m_angPreview;
	float					m_flPreviewDistance;

private:
	static CToyViewer*		s_pToyViewer;
};

inline CToyViewer* ToyViewer()
{
	return CToyViewer::Get();
}
