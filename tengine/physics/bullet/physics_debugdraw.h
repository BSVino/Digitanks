#ifndef TINKER_PHYSICS_DEBUG_DRAWER_H
#define TINKER_PHYSICS_DEBUG_DRAWER_H

#include <LinearMath/btIDebugDraw.h>

#include <common.h>

class CPhysicsDebugDrawer : public btIDebugDraw
{
	DECLARE_CLASS(CPhysicsDebugDrawer, btIDebugDraw);

public:
					CPhysicsDebugDrawer();

public:
	virtual void	SetDrawing(bool bOn) { m_bDrawing = bOn; }

	void            SetRenderingContext(class CRenderingContext* c) { C = c; }

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);

	virtual void	drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

	virtual void	drawSphere(btScalar radius, const btTransform& transform, const btVector3& color);
	virtual void	drawBox(const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha);
	virtual void	drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color);

	virtual void	drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);
	
	virtual void	drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);

	virtual void	reportErrorWarning(const char* warningString);

	virtual void	draw3dText(const btVector3& location, const char* textString);

	virtual void	setDebugMode(int debugMode) { m_iDebugMode = debugMode; };

	virtual int		getDebugMode() const { return m_iDebugMode; }

protected:
	int				m_iDebugMode;
	bool			m_bDrawing;
	class CRenderingContext* C;
};

#endif//GL_DEBUG_DRAWER_H
