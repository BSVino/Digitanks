#pragma once

#include <tstring.h>
#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/cameramanager.h>

#include "tool.h"

class CWorkbenchCamera : public CCameraManager
{
	DECLARE_CLASS(CWorkbenchCamera, CCameraManager);

public:
	virtual void	Think();

	virtual TVector	GetCameraPosition();
	virtual Vector  GetCameraDirection();
	virtual float   GetCameraOrthoHeight();

	virtual bool	ShouldRenderOrthographic();
	virtual bool    UseCustomProjection() { return false; }
};

class CWorkbench : public glgui::IEventListener
{
	friend class CWorkbenchCamera;
	friend class CWorkbenchTool;

public:
	typedef CWorkbenchTool* (*ToolCreator)();

public:
							CWorkbench();
	virtual					~CWorkbench();

public:
	void                    Think();

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, tinker_mouse_state_t iState);
	void					MouseMotion(int x, int y);
	void					MouseWheel(int x, int y);

	void					SetActiveTool(int iTool);

	EVENT_CALLBACK(CWorkbench, MenuSelected);

	CMaterialHandle&        GetArrowTexture() { return m_hArrow; }

public:
	static void				Toggle();
	static bool				IsActive();

	static void				Activate();
	static void				Deactivate();

	static void             LoadLevel(const CHandle<class CLevel>& pLevel);

	static void				RenderScene();

	static CCameraManager*	GetCameraManager();

	static void				RegisterTool(const char* pszTool, ToolCreator pfnToolCreator);

	static CWorkbench*      Get() { return s_pWorkbench; }

protected:
	class CToolRegistration
	{
	public:
		ToolCreator			m_pfnToolCreator;
	};

	static tmap<tstring, CToolRegistration>& GetToolRegistration();

	CWorkbenchTool*			GetActiveTool();

protected:
	bool					m_bActive;
	bool					m_bWasMouseActive;

	CWorkbenchCamera*		m_pCamera;

	tvector<CWorkbenchTool*>	m_apTools;
	size_t					m_iActiveTool;

	glgui::CMenu*			m_pFileMenu;

	CMaterialHandle         m_hArrow;

private:
	static CWorkbench*      s_pWorkbench;
};

CWorkbench* Workbench(bool bCreate=true);

#define REGISTER_WORKBENCH_TOOL(tool) \
class CRegisterWorkbench##tool \
{ \
public: \
	static CWorkbenchTool* Create##tool() \
	{ \
		return new C##tool(); \
	} \
	CRegisterWorkbench##tool() \
	{ \
		CWorkbench::RegisterTool(#tool, &Create##tool); \
	} \
} g_RegisterWorkbench##tool = CRegisterWorkbench##tool(); \
int g_iImport##tool = 0; \

