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
	void							SetupSelector(CScrollSelector<float>* pSelector, float flMaxValue);

	void							Layout();

	EVENT_CALLBACK(CMaterialEditor, ChooseDiffuse);
	EVENT_CALLBACK(CMaterialEditor, ChooseNormal);
	EVENT_CALLBACK(CMaterialEditor, RemoveDiffuse);
	EVENT_CALLBACK(CMaterialEditor, RemoveNormal);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientRed);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientGreen);
	EVENT_CALLBACK(CMaterialEditor, SetAmbientBlue);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseRed);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseGreen);
	EVENT_CALLBACK(CMaterialEditor, SetDiffuseBlue);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularRed);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularGreen);
	EVENT_CALLBACK(CMaterialEditor, SetSpecularBlue);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveRed);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveGreen);
	EVENT_CALLBACK(CMaterialEditor, SetEmissiveBlue);
	EVENT_CALLBACK(CMaterialEditor, SetShininess);

protected:
	CConversionMaterial*			m_pMaterial;
	CSceneTreePanel*				m_pSceneTree;
	CConversionScene*				m_pScene;
	size_t							m_iMaterial;

	CLabel*							m_pDiffuseLabel;
	CButton*						m_pDiffuseFile;
	CButton*						m_pDiffuseRemove;

	CLabel*							m_pNormalLabel;
	CButton*						m_pNormalFile;
	CButton*						m_pNormalRemove;

	CLabel*							m_pAmbientLabel;
	CScrollSelector<float>*			m_pAmbientRedSelector;
	CScrollSelector<float>*			m_pAmbientGreenSelector;
	CScrollSelector<float>*			m_pAmbientBlueSelector;

	CLabel*							m_pDiffuseSelectorLabel;
	CScrollSelector<float>*			m_pDiffuseRedSelector;
	CScrollSelector<float>*			m_pDiffuseGreenSelector;
	CScrollSelector<float>*			m_pDiffuseBlueSelector;

	CLabel*							m_pSpecularLabel;
	CScrollSelector<float>*			m_pSpecularRedSelector;
	CScrollSelector<float>*			m_pSpecularGreenSelector;
	CScrollSelector<float>*			m_pSpecularBlueSelector;

	CLabel*							m_pEmissiveLabel;
	CScrollSelector<float>*			m_pEmissiveRedSelector;
	CScrollSelector<float>*			m_pEmissiveGreenSelector;
	CScrollSelector<float>*			m_pEmissiveBlueSelector;

	CLabel*							m_pShininessLabel;
	CScrollSelector<float>*			m_pShininessSelector;
};

#endif