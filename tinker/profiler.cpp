/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "profiler.h"

#include <strutils.h>

#include <tinker/application.h>
#include <tinker/cvar.h>
#include <renderer/renderingcontext.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>

CVar prof_enable("prof_enable", "no");

CProfileScope::CProfileScope(const char* pszName)
{
	m_pszName = pszName;

	CProfiler::PushScope(this);
}

CProfileScope::~CProfileScope()
{
	CProfiler::PopScope(this);
}

CPerfBlock::CPerfBlock(const char* pszName, CPerfBlock* pParent)
{
	m_pParent = pParent;
	m_pszName = pszName;
	m_flTime = 0;
	m_flLastTextX = 0;
}

CPerfBlock* CPerfBlock::GetChild(const char* pszName)
{
	tmap<const char*, CPerfBlock*>::iterator it = m_apPerfBlocks.find(pszName);

	if (it == m_apPerfBlocks.end())
		return NULL;

	return it->second;
}

CPerfBlock* CPerfBlock::AddChild(const char* pszName)
{
	CPerfBlock* pChild = new CPerfBlock(pszName, this);
	m_apPerfBlocks[pszName] = pChild;
	return pChild;
}

void CPerfBlock::BeginFrame()
{
	m_flTime = 0;

	for (tmap<const char*, CPerfBlock*>::iterator it = m_apPerfBlocks.begin(); it != m_apPerfBlocks.end(); it++)
		it->second->BeginFrame();
}

void CPerfBlock::BlockStarted()
{
	m_flTimeBlockStarted = CApplication::Get()->GetTime();
}

void CPerfBlock::BlockEnded()
{
	double flTimeBlockEnded = CApplication::Get()->GetTime();

	m_flTime += flTimeBlockEnded - m_flTimeBlockStarted;
}

CPerfBlock* CProfiler::s_pTopBlock = NULL;
tvector<CPerfBlock*> CProfiler::s_apBottomBlocks;
bool CProfiler::s_bProfiling = false;
double CProfiler::s_flProfilerTime = 0;
double CProfiler::s_flLastProfilerTime = 0;
double CProfiler::s_flEndLastProfilerTime = 0;

void CProfiler::BeginFrame()
{
	s_bProfiling = prof_enable.GetBool();

	for (auto& pBlock : s_apBottomBlocks)
	{
		if (pBlock)
			pBlock->BeginFrame();
	}

	// Just in case.
	s_pTopBlock = NULL;
}

void CProfiler::PushScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = NULL;

	if (!s_pTopBlock)
	{
		for (auto& pBottomBlock : s_apBottomBlocks)
		{
			if (pBottomBlock && pBottomBlock->GetName() == pScope->GetName())
			{
				pBlock = pBottomBlock;
				break;
			}
		}

		if (!pBlock)
		{
			pBlock = new CPerfBlock(pScope->GetName(), NULL);
			s_apBottomBlocks.push_back(pBlock);
		}
	}
	else
	{
		pBlock = s_pTopBlock->GetChild(pScope->GetName());

		if (!pBlock)
			pBlock = s_pTopBlock->AddChild(pScope->GetName());
	}

	pBlock->BlockStarted();
	s_pTopBlock = pBlock;
}

void CProfiler::PopScope(CProfileScope* pScope)
{
	if (!IsProfiling())
		return;

	TAssert(s_pTopBlock);
	if (!s_pTopBlock)
		return;

	TAssert(pScope->GetName() == s_pTopBlock->GetName());

	s_pTopBlock->BlockEnded();

	s_pTopBlock = s_pTopBlock->GetParent();
}

void CProfiler::PopAllScopes()
{
	if (!IsProfiling())
		return;

	CPerfBlock* pBlock = s_pTopBlock;

	while (pBlock)
	{
		pBlock->BlockEnded();
		pBlock = pBlock->GetParent();
	}

	s_pTopBlock = NULL;
}

void CProfiler::Render()
{
	if (!IsProfiling())
		return;

	if (!s_apBottomBlocks.size())
		return;

	s_flLastProfilerTime = s_flProfilerTime;
	s_flProfilerTime = CApplication::Get()->GetTime();

	PopAllScopes();

	if (prof_enable.GetInt() == 2)
		RenderTimeline();
	else
		RenderTree();

	s_flEndLastProfilerTime = Application()->GetTime();
}

void CProfiler::RenderTree()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flCurrLeft = flWidth - 400;
	float flCurrTop = 10;

	Matrix4x4 mProjection = Matrix4x4::ProjectOrthographic(0, flWidth, flHeight, 0, -1000, 1000);

	CRenderingContext c;

	c.SetProjection(mProjection);
	c.UseProgram("gui");
	c.SetDepthTest(false);
	c.UseFrameBuffer(NULL);

	glgui::CBaseControl::PaintRect(flCurrLeft, flCurrTop, 400, 800, Color(0, 0, 0, 150), 5, true);

	{
		flCurrTop += 15;

		double flProfilerTime = s_flProfilerTime - s_flLastProfilerTime;

		Color clrBlock(255, 255, 255);
		if (flProfilerTime < 0.005)
			clrBlock = Color(255, 255, 255, 150);

		glgui::CBaseControl::PaintRect(flCurrLeft + 15, flCurrTop, (float)flProfilerTime * 5000, 1, clrBlock);

		tstring sName = "Frame time:";
		sName += tsprintf(": %d ms", (int)(flProfilerTime * 1000));
		glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, (float)flCurrLeft + 15, (float)flCurrTop, clrBlock);

		flCurrLeft += 15;
	}

	for (auto& pBlock : s_apBottomBlocks)
		RenderTree(pBlock, flCurrLeft, flCurrTop);

	flCurrTop += 15;
	flCurrLeft += 15;

	double flProfilerTime = CApplication::Get()->GetTime() - s_flProfilerTime;

	Color clrBlock(255, 255, 255);
	if (flProfilerTime < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	glgui::CBaseControl::PaintRect(flCurrLeft, flCurrTop, (float)flProfilerTime * 5000, 1, clrBlock);

	tstring sName = "CProfiler::Render()";
	sName += tsprintf(": %d ms", (int)(flProfilerTime * 1000));
	glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, (float)flCurrLeft, (float)flCurrTop, clrBlock);
}

void CProfiler::RenderTree(CPerfBlock* pBlock, float& flLeft, float& flTop)
{
	flLeft += 15;
	flTop += 15;

	Color clrBlock(255, 255, 255);
	if (pBlock->GetTime() < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	glgui::CBaseControl::PaintRect(flLeft, flTop+1, (float)pBlock->GetTime()*5000, 1, clrBlock);

	tstring sName = pBlock->GetName();
	sName += tsprintf(": %d ms", (int)(pBlock->GetTime()*1000));
	glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, (float)flLeft, (float)flTop, clrBlock);

	for (tmap<const char*, CPerfBlock*>::iterator it = pBlock->m_apPerfBlocks.begin(); it != pBlock->m_apPerfBlocks.end(); it++)
		RenderTree(it->second, flLeft, flTop);

	flLeft -= 15;
}

void RenderTimeline(CPerfBlock* pBlock, double flFrameStart, double flFrameEnd, int iDepth)
{
	Color clrBlock(255, 255, 255);
	if (pBlock->GetTime() < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	float flStart = (float)RemapVal(pBlock->m_flTimeBlockStarted, flFrameStart, flFrameEnd, 0, (double)glgui::RootPanel()->GetWidth());
	float flEnd = (float)RemapVal(pBlock->m_flTimeBlockStarted + pBlock->m_flTime, flFrameStart, flFrameEnd, 0, (double)glgui::RootPanel()->GetWidth());

	if (flEnd - flStart > 1)
	{
		glgui::CBaseControl::PaintRect(flStart, (float)iDepth * 15, flEnd - flStart - 1, 1, clrBlock);

		tstring sName = tsprintf(tstring(pBlock->GetName()) + ": %d ms", (int)(pBlock->GetTime() * 1000));

		float flLength = (float)sName.length() * 7.5f; // A quick approximation.
		if (pBlock->m_flLastTextX > flEnd - flLength)
			pBlock->m_flLastTextX = flEnd - flLength;
		if (pBlock->m_flLastTextX < flStart)
			pBlock->m_flLastTextX = flStart;

		glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, pBlock->m_flLastTextX, (float)iDepth * 15, clrBlock,
			FRect(flStart, (float)iDepth * 15, flEnd - flStart, 15));
	}

	for (tmap<const char*, CPerfBlock*>::iterator it = pBlock->m_apPerfBlocks.begin(); it != pBlock->m_apPerfBlocks.end(); it++)
		RenderTimeline(it->second, flFrameStart, flFrameEnd, iDepth + 1);
}

void CProfiler::RenderTimeline()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	Matrix4x4 mProjection = Matrix4x4::ProjectOrthographic(0, flWidth, flHeight, 0, -1000, 1000);

	CRenderingContext c;

	c.SetProjection(mProjection);
	c.UseProgram("gui");
	c.SetDepthTest(false);
	c.UseFrameBuffer(NULL);

	glgui::CBaseControl::PaintRect(0, 0, flWidth, 150, Color(0, 0, 0, 150), 5, true);

	{
		double flProfilerTime = s_flProfilerTime - s_flLastProfilerTime;

		Color clrBlock(255, 255, 255);
		if (flProfilerTime < 0.005)
			clrBlock = Color(255, 255, 255, 150);

		glgui::CBaseControl::PaintRect(0, 0, glgui::RootPanel()->GetWidth(), 1, clrBlock);

		tstring sName = "Frame time:";
		sName += tsprintf(": %d ms", (int)(flProfilerTime * 1000));
		glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, 0, 0, clrBlock);
	}

	for (auto& pBlock : s_apBottomBlocks)
		::RenderTimeline(pBlock, s_flLastProfilerTime, s_flProfilerTime, 1);

	float flTime = (float)(s_flEndLastProfilerTime - s_flLastProfilerTime);
	float flStart = 0;
	float flEnd = (float)RemapVal(s_flEndLastProfilerTime, s_flLastProfilerTime, s_flProfilerTime, 0, (double)glgui::RootPanel()->GetWidth());

	Color clrBlock(255, 255, 255);
	if (flTime < 0.005)
		clrBlock = Color(255, 255, 255, 150);

	glgui::CBaseControl::PaintRect(flStart, 15, flEnd - flStart - 1, 1, clrBlock);

	tstring sName = tsprintf("Profiler: %d ms", (int)(flTime * 1000));

	glgui::CLabel::PaintText(sName, sName.length(), "sans-serif", 10, flStart, 15, clrBlock,
		FRect(flStart, (float)15, flEnd - flStart, 15));
}
