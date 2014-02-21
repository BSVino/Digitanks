#include "roperenderer.h"

#include <maths.h>
#include <simplex.h>

#include <models/models.h>
#include <renderer/shaders.h>
#include <tinker/application.h>
#include <tinker/cvar.h>
#include <tinker/profiler.h>
#include <game/gameserver.h>
#include <textures/materiallibrary.h>
#include <renderer/renderer.h>

CRopeRenderer::CRopeRenderer(CRenderer *pRenderer, const CMaterialHandle& hMaterial, const Vector& vecStart, float flWidth)
	: m_oContext(pRenderer, true)
{
	m_pRenderer = pRenderer;

	m_oContext.UseMaterial(hMaterial);
	m_vecLastLink = vecStart;
	m_bFirstLink = true;

	m_flWidth = m_flLastLinkWidth = flWidth;

	m_flTextureScale = 1;
	m_flTextureOffset = 0;

	m_bUseForward = false;

	m_clrRope = Color(255, 255, 255, 255);
	m_oContext.BeginRenderTriStrip();
}

void CRopeRenderer::AddLink(const Vector& vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = (vecLink - m_pRenderer->GetCameraPosition()).Normalized();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecRight = (vecForward.Cross(vecUp)).Normalized()*(m_flWidth/2);

	m_oContext.SetColor(m_clrRope);

	if (m_bFirstLink)
	{
		Vector vecLastRight = (m_vecLastLink - m_pRenderer->GetCameraPosition()).Normalized().Cross(vecUp).Normalized()*(m_flLastLinkWidth/2);

		// Do the first two now that we know what right should be.
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
	}

	m_bFirstLink = false;

	m_flTextureOffset += (1/m_flTextureScale);

	m_oContext.TexCoord(1, m_flTextureOffset);
	m_oContext.Vertex(vecLink-vecRight);
	m_oContext.TexCoord(0, m_flTextureOffset);
	m_oContext.Vertex(vecLink+vecRight);

	m_vecLastLink = vecLink;
	m_flLastLinkWidth = m_flWidth;
}

void CRopeRenderer::FinishSegment(const Vector& vecLink, const Vector& vecNextSegmentStart, float flNextSegmentWidth)
{
	// Not updated since the move away from gl2
	TUnimplemented();

	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecLastRight = vecForward.Cross(vecUp)*(m_flLastLinkWidth/2);
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	if (m_bFirstLink)
	{
		// Start the previous link
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}
	else
	{
		m_flTextureOffset += flAddV;

		// Finish the last link
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);

		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}

	m_bFirstLink = true;
	m_vecLastLink = vecNextSegmentStart;
	m_flLastLinkWidth = flNextSegmentWidth;
}

void CRopeRenderer::Finish(const Vector& vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = (vecLink - m_pRenderer->GetCameraPosition()).Normalized();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecRight = vecForward.Cross(vecUp).Normalized()*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	if (m_bFirstLink)
	{
		// Not updated since the move away from gl2
		TUnimplemented();
		Vector vecLastRight = vecForward.Cross(vecUp)*(m_flLastLinkWidth/2);

		// Start the previous link
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecLastRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecLastRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}
	else
	{
		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
	}

	m_oContext.EndRender();
}

void CRopeRenderer::SetForward(const Vector& vecForward)
{
	m_bUseForward = true;
	m_vecForward = vecForward;
}
