#pragma once

#include <tstring.h>
#include <matrix.h>

#include <toys/toy_util.h>

class CConversionScene;
class CConversionMeshInstance;
class CConversionSceneNode;

class CGeppetto
{
public:
				CGeppetto(bool bForce=false, const tstring& sCWD="");

public:
	bool		BuildFiles(const tstring& sOutput, const tstring& sInput, const tstring& sPhysics="", bool bGlobalTransforms = false);
	bool		BuildFromInputScript(const tstring& sScript);

	void		LoadFromFiles(const tstring& sMesh, const tstring& sPhysics);

	void		LoadMeshInstanceIntoToy(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy);
	void		LoadSceneNodeIntoToy(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy);
	void		LoadSceneIntoToy(CConversionScene* pScene, CToyUtil* pToy);

	void		LoadMeshInstanceIntoToyPhysics(CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, const Matrix4x4& mParentTransformations, CToyUtil* pToy);
	void		LoadSceneNodeIntoToyPhysics(CConversionScene* pScene, CConversionSceneNode* pNode, const Matrix4x4& mParentTransformations, CToyUtil* pToy);
	void		LoadSceneIntoToyPhysics(CConversionScene* pScene, CToyUtil* pToy);

	bool		LoadSceneAreas(class CData* pData);

	tstring		GetPath(const tstring& sPath);

protected:
	bool		Compile();

private:
	CToyUtil	t;

	tstring		m_sOutput;

	time_t		m_iBinaryModificationTime;

	bool		m_bForceCompile;

	tstring		m_sCWD;
};
