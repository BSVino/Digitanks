#ifndef CF_CRUNCH_H
#define CF_CRUNCH_H

#include <modelconverter/convmesh.h>
#include "ui/modelwindow.h"

class IWorkListener
{
public:
	virtual void			WorkProgress()=0;
};

typedef enum
{
	AOMETHOD_NONE = 0,
	AOMETHOD_RENDER,
	AOMETHOD_TRIDISTANCE,
	AOMETHOD_RAYTRACE,
	AOMETHOD_SHADOWMAP,
} aomethod_t;

class CAOGenerator
{
public:
							CAOGenerator(CConversionScene* pScene, std::vector<CMaterial>* paoMaterials);
							~CAOGenerator();

public:
	void					SetMethod(aomethod_t eMethod) { m_eAOMethod = eMethod; };
	void					SetSize(size_t iWidth, size_t iHeight);
	void					SetUseTexture(bool bUseTexture) { m_bUseTexture = bUseTexture; };
	void					SetBleed(size_t iBleed) { m_iBleed = iBleed; };
	void					SetRenderPreviewViewport(int x, int y, int w, int h);
	void					SetUseFrontBuffer(bool bUseFrontBuffer) { m_bUseFrontBuffer = bUseFrontBuffer; };
	void					SetSamples(size_t iSamples) { m_iSamples = iSamples; };

	void					SetWorkListener(IWorkListener* pListener) { m_pWorkListener = pListener; };

	void					ShadowMapSetupScene();
	void					RenderSetupScene();
	void					Generate();
	void					GenerateShadowMaps();
	void					GenerateByTexel();
	Vector					RenderSceneFromPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pFace);
	void					DebugRenderSceneLookAtPosition(Vector vecPosition, Vector vecDirection, CConversionFace* pRenderFace);
	void					AccumulateTexture(size_t iTexture);
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
	size_t					m_iSamples;
	int						m_iRPVX;
	int						m_iRPVY;
	int						m_iRPVW;
	int						m_iRPVH;
	bool					m_bUseFrontBuffer;
	aomethod_t				m_eAOMethod;

	IWorkListener*			m_pWorkListener;

	size_t					m_iPixelDepth;
	float*					m_pPixels;
	bool*					m_bPixelMask;

	unsigned int			m_iSceneList;

	Vector*					m_avecShadowValues;
	Vector*					m_avecShadowGeneratedValues;
	size_t*					m_aiShadowReads;
	float					m_flLowestValue;
	float					m_flHighestValue;
	size_t					m_iAOFB;

	bool					m_bIsGenerating;
	bool					m_bDoneGenerating;
	bool					m_bStopGenerating;
};

#endif