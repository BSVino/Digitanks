#ifndef SCENETREE_H
#define SCENETREE_H

#include "modelwindow_ui.h"

class CSceneTreePanel : public CMovablePanel
{
public:
									CSceneTreePanel(CConversionScene* pScene);
									~CSceneTreePanel();

public:
	void							Layout();
	void							Paint(int x, int y, int w, int h);

	void							UpdateTree();
	void							AddAllToTree();
	void							AddNodeToTree(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pNode);

	void							OpenMaterialEditor(CConversionMaterial* pMaterial);

	static void						Open(CConversionScene* pScene);
	static CSceneTreePanel*			Get();

public:
	CConversionScene*				m_pScene;

	modelgui::CTree*				m_pTree;

	class CMaterialEditor*			m_pMaterialEditor;

	static CSceneTreePanel*			s_pSceneTreePanel;
};

void OpenMaterialEditor(CConversionMaterial* pMaterial);

class CMaterialEditor : public CMovablePanel
{
public:
									CMaterialEditor(CConversionMaterial* pMaterial, CSceneTreePanel* pSceneTree);

public:
	void							Layout();

	EVENT_CALLBACK(CMaterialEditor, ChooseDiffuse);

protected:
	CConversionMaterial*			m_pMaterial;
	CSceneTreePanel*				m_pSceneTree;
	CConversionScene*				m_pScene;
	size_t							m_iMaterial;

	CLabel*							m_pDiffuseLabel;
	CLabel*							m_pDiffuseFile;
	CButton*						m_pDiffuseButton;
};

#endif