#pragma once

#include <tengine_config.h>
#include <vector.h>
#include <tstring.h>

#include <tinker/keys.h>
#include <glgui/glgui.h>

namespace glgui
{
	class CMenu;
};

class CWorkbenchTool : public glgui::IEventListener
{
public:
							CWorkbenchTool();
	virtual					~CWorkbenchTool() {};

public:
	virtual void            Think();

	virtual bool			KeyPress(int c) { return false; }
	virtual bool			MouseInput(int iButton, tinker_mouse_state_t iState) { return false; }
	virtual void			MouseMotion(int x, int y) {};
	virtual void			MouseWheel(int x, int y) {};

	virtual void			Activate();
	virtual void			Deactivate();

	virtual void            LoadLevel(const CHandle<class CLevel>& pLevel) {};

	virtual void			RenderScene() {};

	virtual void			CameraThink();
	virtual TVector			GetCameraPosition();
	virtual Vector          GetCameraDirection();
	virtual void			SetCameraOrientation(TVector vecPosition, Vector vecDirection);
	virtual float			GetCameraOrthoHeight();
	virtual bool            ShouldRenderOrthographic();
	virtual bool			ShowCameraControls() { return false; }

	virtual tstring			GetToolName() { return "Invalid Tool"; }

	glgui::CMenu*			GetFileMenu();

	EVENT_CALLBACK(CWorkbenchTool, PerspViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, FrontViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, BackViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, LeftViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, RightViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, TopViewSelected);
	EVENT_CALLBACK(CWorkbenchTool, BottomViewSelected);

protected:
	TVector					m_vecEditCamera;
	EAngle					m_angEditCamera;
	float                   m_flEditOrthoHeight;

	bool                    m_b3DView;
};
