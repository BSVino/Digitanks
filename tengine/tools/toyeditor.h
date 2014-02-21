#pragma once

#include <trs.h>

#include <glgui/movablepanel.h>
#include <tools/manipulator/manipulator.h>

#include "tool.h"

class CCreateToySourcePanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateToySourcePanel, glgui::CMovablePanel);

public:
							CCreateToySourcePanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateToySourcePanel, ToyChanged);
	EVENT_CALLBACK(CCreateToySourcePanel, SourceChanged);
	EVENT_CALLBACK(CCreateToySourcePanel, Create);

	tstring					GetToyFileName();
	tstring					GetSourceFileName();

	void					FileNamesChanged();

public:
	glgui::CLabel*			m_pToyFileLabel;
	glgui::CTextField*		m_pToyFileText;

	glgui::CLabel*			m_pSourceFileLabel;
	glgui::CTextField*		m_pSourceFileText;

	glgui::CLabel*			m_pWarnings;

	glgui::CButton*			m_pCreate;
};

class CToySource
{
public:
	CToySource();

public:
	void    Clear();

	void    Save() const;
	void    Build() const;
	void    Open(const tstring& sFile);

public:
	tstring					m_sFilename;
	tstring					m_sToyFile;

	tstring					m_sMesh;
	tstring					m_sPhys;

	class CPhysicsShape
	{
	public:
		TRS					m_trsTransform;
	};

	tvector<CPhysicsShape>	m_aShapes;

	bool                    m_bUseLocalTransforms;
	float                   m_flNeighborDistance;

	class CSceneArea
	{
	public:
		tstring             m_sName;
		tstring             m_sFilename;
		tstring             m_sMesh;
		tstring             m_sPhys;
	};

	tvector<CSceneArea>     m_aAreas;
};

class CChooseScenePanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CChooseScenePanel, glgui::CMovablePanel);

public:
	CChooseScenePanel(class CSourcePanel* pSourcePanel);

public:
	void Layout();

	void AddSceneToTree(glgui::CTreeNode* pParentTreeNode, class CConversionSceneNode* pSceneNode);

public:
	glgui::CControl<glgui::CTree>   m_hSceneTree;
};

class CSourcePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CSourcePanel, glgui::CPanel);

public:
							CSourcePanel();

public:
	virtual void			SetVisible(bool bVis);

	void					Layout();
	void					LayoutFilename();
	void					UpdateFields();

	void                    SetupChooser(glgui::CControl<glgui::CTextField> hControl);

	void					SetModelSourcesAutoComplete(glgui::CTextField* pField);

	CToySource::CSceneArea* GetCurrentSceneArea() const;

	EVENT_CALLBACK(CSourcePanel, ToyFileChanged);
	EVENT_CALLBACK(CSourcePanel, MeshSource);
	EVENT_CALLBACK(CSourcePanel, MaterialSource);
	EVENT_CALLBACK(CSourcePanel, ModelChanged);
	EVENT_CALLBACK(CSourcePanel, PhysicsChanged);
	EVENT_CALLBACK(CSourcePanel, LocalTransformsChanged);
	EVENT_CALLBACK(CSourcePanel, PhysicsAreaSelected);
	EVENT_CALLBACK(CSourcePanel, NewPhysicsShape);
	EVENT_CALLBACK(CSourcePanel, DeletePhysicsShape);
	EVENT_CALLBACK(CSourcePanel, SceneAreaNameChanged);
	EVENT_CALLBACK(CSourcePanel, SceneAreaMeshSourceChanged);
	EVENT_CALLBACK(CSourcePanel, SceneAreaModelChanged);
	EVENT_CALLBACK(CSourcePanel, SceneAreaPhysicsChanged);
	EVENT_CALLBACK(CSourcePanel, SceneAreaSelected);
	EVENT_CALLBACK(CSourcePanel, SceneAreaMeshChoose);
	EVENT_CALLBACK(CSourcePanel, SceneAreaPhysChoose);
	EVENT_CALLBACK(CSourcePanel, NewSceneArea);
	EVENT_CALLBACK(CSourcePanel, DeleteSceneArea);
	EVENT_CALLBACK(CSourcePanel, Save);
	EVENT_CALLBACK(CSourcePanel, Build);
	EVENT_CALLBACK(CSourcePanel, SceneAreaConfirmed);

public:
	glgui::CControl<glgui::CLabel>     m_hFilename;

	glgui::CControl<glgui::CLabel>     m_hToyFileLabel;
	glgui::CControl<glgui::CTextField> m_hToyFileText;

	glgui::CControl<glgui::CMenu>      m_hMeshMenu;
	bool                               m_bMesh;
	glgui::CControl<glgui::CTextField> m_hMeshText;

	glgui::CControl<glgui::CLabel>     m_hPhysLabel;
	glgui::CControl<glgui::CTextField> m_hPhysText;

	glgui::CControl<glgui::CCheckBox>  m_hUseLocalTransformsCheck;
	glgui::CControl<glgui::CLabel>     m_hUseLocalTransformsLabel;

	glgui::CControl<glgui::CSlidingContainer> m_hSlider;
	glgui::CControl<glgui::CSlidingPanel>     m_hPhysicsSlider;
	glgui::CControl<glgui::CSlidingPanel>     m_hAreasSlider;

	glgui::CControl<glgui::CTree>   m_hPhysicsShapes;
	glgui::CControl<glgui::CButton> m_hNewPhysicsShape;
	glgui::CControl<glgui::CButton> m_hDeletePhysicsShape;

	glgui::CControl<glgui::CTree>   m_hAreas;
	glgui::CControl<glgui::CButton> m_hNewArea;
	glgui::CControl<glgui::CButton> m_hDeleteArea;

	glgui::CControl<glgui::CLabel>     m_hAreaNameLabel;
	glgui::CControl<glgui::CTextField> m_hAreaNameText;

	glgui::CControl<glgui::CLabel>     m_hAreaSourceFileLabel;
	glgui::CControl<glgui::CTextField> m_hAreaSourceFileText;

	glgui::CControl<glgui::CLabel>     m_hAreaMeshLabel;
	glgui::CControl<glgui::CTextField> m_hAreaMeshText;
	glgui::CControl<glgui::CButton>    m_hAreaMeshChoose;

	glgui::CControl<glgui::CLabel>     m_hAreaPhysLabel;
	glgui::CControl<glgui::CTextField> m_hAreaPhysText;
	glgui::CControl<glgui::CButton>    m_hAreaPhysChoose;

	glgui::CControl<glgui::CButton> m_hSave;
	glgui::CControl<glgui::CButton> m_hBuild;

	glgui::CControl<CChooseScenePanel> m_hSceneChooser;
	glgui::CControl<glgui::CTextField> m_hChoosingTextField;
};

class CToyEditor : public CWorkbenchTool, public IManipulatorListener
{
	DECLARE_CLASS(CToyEditor, CWorkbenchTool);

public:
							CToyEditor();
	virtual					~CToyEditor();

public:
	virtual void			Activate();
	virtual void			Deactivate();

	void					Layout();
	void                    ReloadModels();
	void					SetupMenu();

	virtual void			RenderScene();

	void					NewToy();
	CToySource&				GetToyToModify();
	const CToySource&		GetToy() { return m_oToySource; }
	void					MarkUnsaved();
	void					MarkSaved();
	bool					IsSaved() { return m_bSaved; }

	EVENT_CALLBACK(CToyEditor, NewToy);
	EVENT_CALLBACK(CToyEditor, SaveToy);
	EVENT_CALLBACK(CToyEditor, ChooseToy);
	EVENT_CALLBACK(CToyEditor, OpenToy);
	EVENT_CALLBACK(CToyEditor, BuildToy);

	bool					KeyPress(int c);
	bool					MouseInput(int iButton, tinker_mouse_state_t iState);
	void					MouseMotion(int x, int y);
	void					MouseWheel(int x, int y);

	virtual TVector			GetCameraPosition();
	virtual Vector          GetCameraDirection();

	virtual void			ManipulatorUpdated(const tstring& sArguments);
	virtual void            DuplicateMove(const tstring& sArguments);

	virtual CSourcePanel*   GetSourcePanel() const { return m_pSourcePanel; }

	// Don't store the result.
	class CConversionScene* FindLoadedSceneFromFile(const tstring& sFile);

	void                    ReloadPreview() { m_bReloadPreview = true; }

	virtual tstring			GetToolName() { return "Toy Editor"; }

public:
	static CToyEditor*		Get() { return s_pToyEditor; }

protected:
	CCreateToySourcePanel*	m_pCreateToySourcePanel;

	CSourcePanel*			m_pSourcePanel;

	CToySource				m_oToySource;

	size_t					m_iMeshPreview;
	size_t					m_iPhysPreview;
	CMaterialHandle			m_hMaterialPreview;
	bool                    m_bReloadPreview;

	// Holds the scene and GL indexes for all of the scene areas.
	class CSceneAreaModelData
	{
	public:
		std::shared_ptr<class CConversionScene> pScene;
		tvector<size_t>                         aiModels;
		tvector<CMaterialHandle>                ahMaterials;
		bool                                    bMark;
	};

	tmap<tstring, CSceneAreaModelData> m_aFileScenes;

	bool					m_bRotatingPreview;
	EAngle					m_angPreview;
	bool					m_bDollyingPreview;
	float					m_flPreviewDistance;

	bool					m_bSaved;

private:
	static CToyEditor*		s_pToyEditor;
};

inline CToyEditor* ToyEditor()
{
	return CToyEditor::Get();
}
