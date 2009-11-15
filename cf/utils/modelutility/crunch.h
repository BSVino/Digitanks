#ifndef CF_CRUNCH_H
#define CF_CRUNCH_H

#include <modelconverter/convmesh.h>
#include "ui/modelwindow.h"

#include <GL/glut.h>

class IWorkListener
{
public:
	virtual void			WorkProgress()=0;
};

class CColorAOGenerator
{
public:
							CColorAOGenerator(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
							~CColorAOGenerator();

	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };
	void					SetBleed(size_t iBleed) { m_iBleed = iBleed; };
	void					SetRenderPreviewViewport(int x, int y, int w, int h);
	void					SetUseFrontBuffer(bool bUseFrontBuffer) { m_bUseFrontBuffer = bUseFrontBuffer; };

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					Generate();
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);
	void					Bleed();

	size_t					GenerateTexture();
	void					SaveToFile(const wchar_t* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);

	bool					HasGenerated() { return m_bHasGenerated; }

protected:
	CConversionScene*		m_pScene;
	std::vector<CMaterial>*	m_paoMaterials;

	size_t					m_iWidth;
	size_t					m_iHeight;
	bool					m_bUseTexture;
	size_t					m_iBleed;
	int						m_iRPVX;
	int						m_iRPVY;
	int						m_iRPVW;
	int						m_iRPVH;
	bool					m_bUseFrontBuffer;

	IWorkListener*			m_pWorkListener;

	size_t					m_iPixelDepth;
	GLfloat*				m_pPixels;
	bool*					m_bPixelMask;

	GLuint					m_iSceneList;

	Vector*					m_avecShadowValues;
	size_t*					m_aiShadowReads;

	bool					m_bHasGenerated;
};

#endif