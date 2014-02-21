///////////////////////////////////////////////////////////////////////////////////////////////////
// LibGizmo
// File Name : 
// Creation : 10/01/2012
// Author : Cedric Guillemet
// Description : LibGizmo
//
///Copyright (C) 2012 Cedric Guillemet
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//of the Software, and to permit persons to whom the Software is furnished to do
///so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 


#include "GizmoTransformRender.h"

#include <tinker/renderer/renderingcontext.h>

void CGizmoTransformRender::DrawCircle(class CRenderingContext* c, const tvector3 &orig,float r,float g,float b,const tvector3 &vtx,const tvector3 &vty)
{
	c->SetDepthTest(false);
	c->SetUniform("vecColor", Vector4D(r, g, b, 1));

	c->SetBlend(BLEND_NONE);
	c->SetBackCulling(true);

	c->BeginRenderLineLoop();
	for (int i = 0; i < 50 ; i++)
	{
		tvector3 vt;
		vt = vtx * cos((2*ZPI/50)*i);
		vt += vty * sin((2*ZPI/50)*i);
		vt += orig;
		c->Vertex(Vector(vt.x,vt.y,vt.z));
	}
	c->EndRender();
}


void CGizmoTransformRender::DrawCircleHalf(class CRenderingContext* c, const tvector3 &orig,float r,float g,float b,const tvector3 &vtx,const tvector3 &vty,tplane &camPlan)
{
	c->SetDepthTest(false);
	c->SetUniform("vecColor", Vector4D(r, g, b, 1));

	c->SetBlend(BLEND_NONE);
	c->SetBackCulling(true);

	c->BeginRenderLineStrip();
	for (int i = 0; i < 30 ; i++)
	{
		tvector3 vt;
		vt = vtx * cos((ZPI/30)*i);
		vt += vty * sin((ZPI/30)*i);
		vt +=orig;
		if (camPlan.DotNormal(vt))
			c->Vertex(Vector(vt.x,vt.y,vt.z));
	}
	c->EndRender();
}

void CGizmoTransformRender::DrawAxis(class CRenderingContext* c, const tvector3 &orig, const tvector3 &axis, const tvector3 &vtx,const tvector3 &vty, float fct,float fct2,const tvector4 &col)
{
	c->SetDepthTest(false);
	c->SetUniform("vecColor", Vector4D(&col.x));

	c->SetBlend(BLEND_NONE);
	c->SetBackCulling(true);

	c->BeginRenderLines();
		c->Vertex(Vector(&orig.x));
		c->Vertex(Vector(orig.x+axis.x,orig.y+axis.y,orig.z+axis.z));
	c->EndRender();

	c->BeginRenderTriFan();
	for (int i=0;i<=30;i++)
	{
		tvector3 pt;
		pt = vtx * cos(((2*ZPI)/30.0f)*i)*fct;
		pt+= vty * sin(((2*ZPI)/30.0f)*i)*fct;
		pt+=axis*fct2;
		pt+=orig;
		c->Vertex(&pt.x);
		pt = vtx * cos(((2*ZPI)/30.0f)*(i+1))*fct;
		pt+= vty * sin(((2*ZPI)/30.0f)*(i+1))*fct;
		pt+=axis*fct2;
		pt+=orig;
		c->Vertex(&pt.x);
		c->Vertex(Vector(orig.x+axis.x,orig.y+axis.y,orig.z+axis.z));
	}
	c->EndRender();
}

void CGizmoTransformRender::DrawAxisScale(class CRenderingContext* c, const tvector3 &orig, const tvector3 &axis, const tvector3 &vtx,const tvector3 &vty, float fct,float fct2,const tvector4 &col)
{
	c->SetDepthTest(false);
	c->SetUniform("vecColor", Vector4D(&col.x));

	c->SetBlend(BLEND_NONE);
	c->SetBackCulling(true);

	c->BeginRenderLines();
		c->Vertex(Vector(&orig.x));
		c->Vertex(Vector(orig.x+axis.x,orig.y+axis.y,orig.z+axis.z));
	c->EndRender();

	float flSize = axis.Length() * fct;

	Vector vecEnd = Vector(orig.x+axis.x,orig.y+axis.y,orig.z+axis.z) - Vector(axis).Normalized()*(flSize/2);

	c->BeginRenderTriFan();
		c->Vertex(vecEnd + Vector(flSize, flSize, flSize));
		c->Vertex(vecEnd + Vector(-flSize, flSize, flSize));
		c->Vertex(vecEnd + Vector(-flSize, -flSize, flSize));
		c->Vertex(vecEnd + Vector(flSize, -flSize, flSize));
		c->Vertex(vecEnd + Vector(flSize, -flSize, -flSize));
		c->Vertex(vecEnd + Vector(flSize, flSize, -flSize));
		c->Vertex(vecEnd + Vector(-flSize, flSize, -flSize));
		c->Vertex(vecEnd + Vector(-flSize, flSize, flSize));
	c->EndRender();

	c->BeginRenderTriFan();
		c->Vertex(vecEnd + Vector(-flSize, -flSize, -flSize));
		c->Vertex(vecEnd + Vector(flSize, -flSize, -flSize));
		c->Vertex(vecEnd + Vector(flSize, -flSize, flSize));
		c->Vertex(vecEnd + Vector(-flSize, -flSize, flSize));
		c->Vertex(vecEnd + Vector(-flSize, flSize, flSize));
		c->Vertex(vecEnd + Vector(-flSize, flSize, -flSize));
		c->Vertex(vecEnd + Vector(flSize, flSize, -flSize));
		c->Vertex(vecEnd + Vector(flSize, -flSize, -flSize));
	c->EndRender();
}

void CGizmoTransformRender::DrawCamem(class CRenderingContext* c, const tvector3& orig,const tvector3& vtx,const tvector3& vty,float ng)
{
	c->SetDepthTest(false);
	c->SetUniform("vecColor", Vector4D(1, 1, 0, 0.5f));

	c->SetBlend(BLEND_ALPHA);
	c->SetBackCulling(false);

	c->BeginRenderTriFan();
	c->Vertex(&orig.x);
	for (int i = 0 ; i <= 50 ; i++)
	{
		tvector3 vt;
		vt = vtx * cos(((ng)/50)*i);
		vt += vty * sin(((ng)/50)*i);
		vt+=orig;
		c->Vertex(Vector(vt.x,vt.y,vt.z));
	}
	c->EndRender();

	c->SetBlend(BLEND_NONE);
	c->SetUniform("vecColor", Vector4D(1, 1, 0.2f, 1));

	tvector3 vt;
	c->BeginRenderLineLoop();

	c->Vertex(&orig.x);
	for (int i = 0 ; i <= 50 ; i++)
	{
		tvector3 vt;
		vt = vtx * cos(((ng)/50)*i);
		vt += vty * sin(((ng)/50)*i);
		vt+=orig;
		c->Vertex(Vector(vt.x,vt.y,vt.z));
	}
	c->EndRender();
}

void CGizmoTransformRender::DrawQuad(class CRenderingContext* c, const tvector3& orig, float size, bool bSelected, const tvector3& axisU, const tvector3 &axisV)
{
	c->SetDepthTest(false);

	c->SetBlend(BLEND_ALPHA);
	c->SetBackCulling(false);

	tvector3 pts[4];
	pts[0] = orig;
	pts[1] = orig + (axisU * size);
	pts[2] = orig + (axisU + axisV)*size;
	pts[3] = orig + (axisV * size);

	if (!bSelected)
		c->SetUniform("vecColor", Vector4D(1, 1, 0, 0.5f));
	else
		c->SetUniform("vecColor", Vector4D(1, 1, 1, 0.6f));

	c->BeginRenderTriFan();
		c->Vertex(&pts[0].x);
		c->Vertex(&pts[1].x);
		c->Vertex(&pts[2].x);
		c->Vertex(&pts[3].x);
	c->EndRender();

	if (!bSelected)
		c->SetUniform("vecColor", Vector4D(1, 1, 0.2f, 1));
	else
		c->SetUniform("vecColor", Vector4D(1, 1, 1, 0.6f));

	c->BeginRenderLineStrip();
		c->Vertex(&pts[0].x);
		c->Vertex(&pts[1].x);
		c->Vertex(&pts[2].x);
		c->Vertex(&pts[3].x);
	c->EndRender();
}


void CGizmoTransformRender::DrawTri(class CRenderingContext* c, const tvector3& orig, float size, bool bSelected, const tvector3& axisU, const tvector3& axisV)
{
	c->SetDepthTest(false);

	c->SetBlend(BLEND_ALPHA);
	c->SetBackCulling(false);

	tvector3 pts[3];
	pts[0] = orig;

	pts[1] = (axisU );
	pts[2] = (axisV );

	pts[1]*=size;
	pts[2]*=size;
	pts[1]+=orig;
	pts[2]+=orig;

	if (!bSelected)
		c->SetUniform("vecColor", Vector4D(1, 1, 0, 0.5f));
	else
		c->SetUniform("vecColor", Vector4D(1, 1, 1, 0.6f));

	c->BeginRenderTris();
		c->Vertex(&pts[0].x);
		c->Vertex(&pts[1].x);
		c->Vertex(&pts[2].x);
		c->Vertex(&pts[3].x);
	c->EndRender();

	if (!bSelected)
		c->SetUniform("vecColor", Vector4D(1, 1, 0.2f, 1));
	else
		c->SetUniform("vecColor", Vector4D(1, 1, 1, 0.6f));

	c->BeginRenderLineStrip();
		c->Vertex(&pts[0].x);
		c->Vertex(&pts[1].x);
		c->Vertex(&pts[2].x);
	c->EndRender();
}
