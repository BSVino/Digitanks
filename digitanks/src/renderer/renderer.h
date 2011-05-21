#ifndef DT_RENDERER_H
#define DT_RENDERER_H

#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <vector.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

typedef enum
{
	BLEND_NONE = 0,
	BLEND_ALPHA,
	BLEND_ADDITIVE,
} blendtype_t;

class CRenderingContext
{
public:
				CRenderingContext(class CRenderer* pRenderer);
				~CRenderingContext();

public:
	void		Transform(const Matrix4x4& m);
	void		Translate(Vector vecTranslate);
	void		Rotate(float flAngle, Vector vecAxis);
	void		Scale(float flX, float flY, float flZ);
	void		ResetTransformations();

	void		SetBlend(blendtype_t eBlend);
	void		SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };
	void		SetDepthMask(bool bDepthMask);
	void		SetDepthTest(bool bDepthTest);
	void		SetBackCulling(bool bCull);
	void		SetColorSwap(Color clrSwap);

	float		GetAlpha() { return m_flAlpha; };
	blendtype_t	GetBlend() { return m_eBlend; };

	void		RenderModel(size_t iModel, class CModel* pCompilingModel = NULL);
	void		RenderSceneNode(class CModel* pModel, class CConversionScene* pScene, class CConversionSceneNode* pNode, class CModel* pCompilingModel);
	void		RenderMeshInstance(class CModel* pModel, class CConversionScene* pScene, class CConversionMeshInstance* pMeshInstance, class CModel* pCompilingModel);

	void		RenderSphere();

	void		UseFrameBuffer(const class CFrameBuffer* pBuffer);
	void		UseProgram(size_t iProgram);
	void		SetUniform(const char* pszName, int iValue);
	void		SetUniform(const char* pszName, float flValue);
	void		SetUniform(const char* pszName, const Vector& vecValue);
	void		SetUniform(const char* pszName, const Color& vecValue);
	void		BindTexture(size_t iTexture);
	void		SetColor(Color c);
	void		BeginRenderTris();
	void		BeginRenderQuads();
	void		TexCoord(float s, float t);
	void		TexCoord(const Vector& v);
	void		Vertex(const Vector& v);
	void		RenderCallList(size_t iCallList);
	void		EndRender();

protected:
	void		PushAttribs();

public:
	CRenderer*	m_pRenderer;

	bool		m_bMatrixTransformations;
	bool		m_bBoundTexture;
	bool		m_bFBO;
	size_t		m_iProgram;
	bool		m_bAttribs;

	bool		m_bColorSwap;
	Color		m_clrSwap;

	blendtype_t	m_eBlend;
	float		m_flAlpha;
};

class CRopeRenderer
{
public:
						CRopeRenderer(class CRenderer* pRenderer, size_t iTexture, Vector vecStart, float flWidth);

public:
	void				AddLink(Vector vecLink);
	void				FinishSegment(Vector vecLink, Vector vecNextSegmentStart, float flNextSegmentWidth);
	void				Finish(Vector vecLink);

	void				SetWidth(float flWidth) { m_flWidth = flWidth; };
	void				SetColor(Color c) { m_clrRope = c; };
	void				SetTextureScale(float flTextureScale) { m_flTextureScale = flTextureScale; };
	void				SetTextureOffset(float flTextureOffset) { m_flTextureOffset = flTextureOffset; };

	void				SetForward(Vector vecForward);

protected:
	CRenderer*			m_pRenderer;
	CRenderingContext	m_oContext;

	size_t				m_iTexture;
	Vector				m_vecLastLink;
	float				m_flLastLinkWidth;
	bool				m_bFirstLink;

	float				m_flWidth;
	Color				m_clrRope;
	float				m_flTextureScale;
	float				m_flTextureOffset;

	bool				m_bUseForward;
	Vector				m_vecForward;
};

class CFrameBuffer
{
public:
				CFrameBuffer();

public:
	size_t		m_iWidth;
	size_t		m_iHeight;

	size_t		m_iMap;
	size_t		m_iDepth;
	size_t		m_iFB;

	size_t		m_iCallList;
};

class CRenderBatch
{
public:
	class CModel*	pModel;
	Matrix4x4		mTransformation;
	bool			bSwap;
	Color			clrSwap;
};

#define BLOOM_FILTERS 3

class CRenderer
{
public:
					CRenderer(size_t iWidth, size_t iHeight);

public:
	virtual void	Initialize();

	CFrameBuffer	CreateFrameBuffer(size_t iWidth, size_t iHeight, bool bDepth, bool bLinear);

	void			CreateNoise();

	virtual void	SetupFrame();
	virtual void	DrawBackground();
	virtual void	StartRendering();
	virtual void	FinishRendering();
	virtual void	RenderOffscreenBuffers() {};
	virtual void	RenderFullscreenBuffers() {};
	virtual void	SetupSceneShader() {};

	void			RenderMapFullscreen(size_t iMap);
	void			RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer);

	void			SetCameraPosition(Vector vecCameraPosition) { m_vecCameraPosition = vecCameraPosition; };
	void			SetCameraTarget(Vector vecCameraTarget) { m_vecCameraTarget = vecCameraTarget; };
	void			SetCameraFOV(float flFOV) { m_flCameraFOV = flFOV; };
	void			SetCameraNear(float flNear) { m_flCameraNear = flNear; };
	void			SetCameraFar(float flFar) { m_flCameraFar = flFar; };

	Vector			GetCameraPosition() { return m_vecCameraPosition; };
	Vector			GetCameraTarget() { return m_vecCameraTarget; };
	float			GetCameraFOV() { return m_flCameraFOV; };
	float			GetCameraNear() { return m_flCameraNear; };
	float			GetCameraFar() { return m_flCameraFar; };

	void			FrustumOverride(Vector vecPosition, Vector vecTarget, float flFOV, float flNear, float flFar);
	void			CancelFrustumOverride();

	bool			ShouldBatchThisFrame() { return m_bBatchThisFrame; }
	void			BeginBatching();
	void			AddToBatch(class CModel* pModel, const Matrix4x4& mTransformations, bool bClrSwap, const Color& clrSwap);
	void			RenderBatches();
	bool			IsBatching() { return m_bBatching; };

	Vector			GetCameraVector();
	void			GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp);

	bool			IsSphereInFrustum(Vector vecCenter, float flRadius);

	void			SetSize(int w, int h);

	void			ClearProgram();
	void			UseProgram(size_t i);

	Vector			ScreenPosition(Vector vecWorld);
	Vector			WorldPosition(Vector vecScreen);

	const CFrameBuffer*	GetSceneBuffer() { return &m_oSceneBuffer; }

	bool			ShouldUseFramebuffers() { return m_bUseFramebuffers; }
	bool			HardwareSupportsFramebuffers();

	bool			ShouldUseShaders() { return m_bUseShaders; }
	bool			HardwareSupportsShaders();

public:
	static size_t	CreateCallList(size_t iModel);
	static size_t	LoadTextureIntoGL(eastl::string16 sFilename, int iClamp = 0);
	static size_t	LoadTextureIntoGL(size_t iImageID, int iClamp = 0);
	static size_t	LoadTextureIntoGL(Color* pclrData, int iClamp = 0);
	static void		UnloadTextureFromGL(size_t iGLID);
	static size_t	GetNumTexturesLoaded() { return s_iTexturesLoaded; }

	static size_t	LoadTextureData(eastl::string16 sFilename);
	static Color*	GetTextureData(size_t iTexture);
	static size_t	GetTextureWidth(size_t iTexture);
	static size_t	GetTextureHeight(size_t iTexture);
	static void		UnloadTextureData(size_t iTexture);

protected:
	size_t			m_iWidth;
	size_t			m_iHeight;

	Vector			m_vecCameraPosition;
	Vector			m_vecCameraTarget;
	float			m_flCameraFOV;
	float			m_flCameraNear;
	float			m_flCameraFar;

	bool			m_bFrustumOverride;
	Vector			m_vecFrustumPosition;
	Vector			m_vecFrustumTarget;
	float			m_flFrustumFOV;
	float			m_flFrustumNear;
	float			m_flFrustumFar;

	double			m_aiModelView[16];
	double			m_aiProjection[16];
	int				m_aiViewport[4];

	Plane			m_aoFrustum[6];

	bool			m_bBatchThisFrame;
	bool			m_bBatching;
	eastl::map<size_t, eastl::vector<CRenderBatch> > m_aBatches;

	CFrameBuffer	m_oSceneBuffer;

	CFrameBuffer	m_oBloom1Buffers[BLOOM_FILTERS];
	CFrameBuffer	m_oBloom2Buffers[BLOOM_FILTERS];

	CFrameBuffer	m_oNoiseBuffer;

	bool			m_bUseFramebuffers;
	bool			m_bHardwareSupportsFramebuffers;
	bool			m_bHardwareSupportsFramebuffersTestCompleted;

	bool			m_bUseShaders;
	bool			m_bHardwareSupportsShaders;
	bool			m_bHardwareSupportsShadersTestCompleted;

	static size_t	s_iTexturesLoaded;
};

#endif
