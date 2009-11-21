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

typedef enum
{
	AOMETHOD_RENDER,
	AOMETHOD_TRIDISTANCE,
} aomethod_t;

class CAOGenerator
{
public:
							CAOGenerator(aomethod_t eMethod, CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
							~CAOGenerator();

	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };
	void					SetBleed(size_t iBleed) { m_iBleed = iBleed; };
	void					SetRenderPreviewViewport(int x, int y, int w, int h);
	void					SetUseFrontBuffer(bool bUseFrontBuffer) { m_bUseFrontBuffer = bUseFrontBuffer; };

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					RenderSetupScene();
	void					Generate();
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);
	void					Bleed();

	size_t					GenerateTexture(bool bInMedias = false);
	void					SaveToFile(const wchar_t* pszFilename);

	bool					Texel(size_t w, size_t h, size_t& iTexel, bool bUseMask = true);

	bool					IsGenerating() { return m_bIsGenerating; }
	bool					DoneGenerating() { return m_bDoneGenerating; }
	void					StopGenerating() { m_bStopGenerating = true; }

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
	aomethod_t				m_eAOMethod;

	IWorkListener*			m_pWorkListener;

	size_t					m_iPixelDepth;
	GLfloat*				m_pPixels;
	bool*					m_bPixelMask;

	GLuint					m_iSceneList;

	Vector*					m_avecShadowValues;
	size_t*					m_aiShadowReads;
	float					m_flLowestValue;
	float					m_flHighestValue;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;
};

#endif