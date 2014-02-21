#pragma once

#include <tstring.h>
#include <tinker_memory.h>

#include <glgui/panel.h>
#include <glgui/movablepanel.h>
#include <game/cameramanager.h>
#include <game/level.h>
#include <tools/manipulator/manipulator.h>

#include "../tool.h"

class CEditorPanel;
class CCreateEntityPanel;

class CLevelEditor : public CWorkbenchTool, public IManipulatorListener
{
	DECLARE_CLASS(CLevelEditor, CWorkbenchTool);

public:
							CLevelEditor();
	virtual					~CLevelEditor();

public:
	virtual void            LoadLevel(const CHandle<class CLevel>& pLevel);

	virtual void            Think();

	void					RenderEntity(size_t i);
	void					RenderEntity(class CLevelEntity* pEntity, bool bSelected=false, bool bHover=false);
	void					RenderEntity(class CLevelEntity* pEntity, bool bSelected, bool bHover, const Matrix4x4& mGlobal);
	void					RenderCreateEntityPreview();

	Vector					PositionFromMouse();
	Vector                  DirectionFromMouse();
	size_t                  TraceLine(const Ray& vecTrace);
	void					EntitySelected();
	void					CreateEntityFromPanel(const Vector& vecPosition);
	static void				PopulateLevelEntityFromPanel(class CLevelEntity* pEntity, class CEntityPropertiesPanel* pPanel);
	void					DuplicateSelectedEntity();

	class CLevel*			GetLevel() { return m_pLevel; }

	EVENT_CALLBACK(CLevelEditor, CreateEntity);
	EVENT_CALLBACK(CLevelEditor, SaveLevel);

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, tinker_mouse_state_t iState);

	virtual void			Activate();
	virtual void			Deactivate();

	virtual void			RenderScene();

	virtual void			ManipulatorUpdated(const tstring& sArguments);
	virtual void            DuplicateMove(const tstring& sArguments);

	virtual bool			ShowCameraControls() { return true; }

	virtual tstring			GetToolName() { return "Level Editor"; }

	static CLevelEditor*	Get() { return s_pLevelEditor; }

protected:
	CHandle<class CLevel>	m_pLevel;

	glgui::CControl<CEditorPanel>			m_hEditorPanel;

	glgui::CControl<glgui::CPictureButton>	m_hCreateEntityButton;
	glgui::CControl<CCreateEntityPanel>		m_hCreateEntityPanel;

	float					m_flCreateObjectDistance;

	size_t					m_iHoverEntity;

private:
	static CLevelEditor*	s_pLevelEditor;
};

inline CLevelEditor* LevelEditor()
{
	return CLevelEditor::Get();
}
