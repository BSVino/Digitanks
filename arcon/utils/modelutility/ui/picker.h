#ifndef PICKER_H
#define PICKER_H

#include "modelwindow_ui.h"

class CPicker : public CMovablePanel
{
public:
								CPicker(char* pszName, IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Delete() { delete this; };

public:
	virtual void				Layout();

	virtual void				Think();

	virtual void				PopulateTree() {};

	EVENT_CALLBACK(CPicker,		Selected);

	virtual void				NodeSelected(CTreeNode* pNode) {};

protected:
	virtual void				Open();
	virtual void				Close();

	IEventListener::Callback	m_pfnCallback;
	IEventListener*				m_pCallback;

	CTree*						m_pTree;

	bool						m_bPopulated;
};

class CMeshInstancePicker : public CPicker
{
public:
								CMeshInstancePicker(IEventListener* pCallback, IEventListener::Callback pfnCallback);

public:
	virtual void				Delete() { delete this; };

public:
	virtual void				PopulateTree();
	virtual void				PopulateTreeNode(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode);

	virtual void				NodeSelected(CTreeNode* pNode);

	virtual CConversionMeshInstance*	GetPickedMeshInstance() { return m_pPickedMeshInstance; }

protected:
	CConversionMeshInstance*	m_pPickedMeshInstance;
};

#endif
