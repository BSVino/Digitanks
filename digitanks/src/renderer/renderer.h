#ifndef DT_RENDERER_H
#define DT_RENDERER_H

#include <vector.h>

class CRenderingContext
{
public:
				CRenderingContext();
				~CRenderingContext();

public:
	void		Translate(Vector vecTranslate);
	void		Rotate(float flAngle, Vector vecAxis);
	void		Scale(float flX, float flY, float flZ);
	void		SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };

	void		RenderModel(size_t iModel);
	void		RenderSceneNode(class CConversionScene* pScene, class CConversionSceneNode* pNode);
	void		RenderMeshInstance(class CConversionScene* pScene, class CConversionMeshInstance* pMeshInstance);

public:
	bool		m_bMatrixTransformations;

	float		m_flAlpha;
};

#endif
