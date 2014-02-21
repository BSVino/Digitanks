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


#include "GizmoTransform.h"

#include <tinker/renderer/renderer.h>

#include "manipulator.h"

void CGizmoTransform::BuildRay(int x, int y, tvector3 &rayOrigin, tvector3 &rayDir)
{
	float frameX = (float)mScreenWidth;
	float frameY = (float)mScreenHeight;
	tvector3 screen_space;

	// device space to normalized screen space
	screen_space.x = ( ( (2.f * (float)x) / frameX ) - 1 ) / m_Proj.m[0][0];//.right.x;
	screen_space.y = -( ( (2.f * (float)y) / frameY ) - 1 ) / m_Proj.m[1][1];
	screen_space.z = -1.f;

	// screen space to world space

	rayOrigin = m_invmodel.V4.position;
	rayDir.TransformVector(screen_space, m_invmodel);
	rayDir.Normalize();

	if (Manipulator()->GetRenderer() && Manipulator()->GetRenderer()->ShouldRenderOrthographic())
	{
		rayOrigin = tvector3(Manipulator()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 0)));
		rayDir = tvector3((Manipulator()->GetRenderer()->WorldPosition(Vector((float)x, (float)y, 1)) - Vector(rayOrigin)).Normalized());
	}
}

float CGizmoTransform::GetScreenFactor()
{
	if (Manipulator()->GetRenderer() && Manipulator()->GetRenderer()->ShouldRenderOrthographic())
		return m_ScreenFactor / m_Proj.m[0][0];

	return m_ScreenFactor;
}
