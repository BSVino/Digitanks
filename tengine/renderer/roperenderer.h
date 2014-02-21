#ifndef TINKER_ROPERENDERER_H
#define TINKER_ROPERENDERER_H

#include <tstring.h>
#include <vector.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

#include <renderer/render_common.h>
#include <renderer/renderingcontext.h>
#include <textures/materialhandle.h>

class CRopeRenderer
{
public:
						CRopeRenderer(class CRenderer* pRenderer, const CMaterialHandle& hMaterial, const Vector& vecStart, float flWidth);

public:
	void				AddLink(const Vector& vecLink);
	void				FinishSegment(const Vector& vecLink, const Vector& vecNextSegmentStart, float flNextSegmentWidth);
	void				Finish(const Vector& vecLink);

	void				SetWidth(float flWidth) { m_flWidth = flWidth; };
	void				SetColor(Color c) { m_clrRope = c; };
	void				SetTextureScale(float flTextureScale) { m_flTextureScale = flTextureScale; };
	void				SetTextureOffset(float flTextureOffset) { m_flTextureOffset = flTextureOffset; };

	void				SetForward(const Vector& vecForward);

protected:
	CRenderer*			m_pRenderer;
	CRenderingContext	m_oContext;

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

#endif
