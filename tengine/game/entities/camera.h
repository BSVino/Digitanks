#pragma once

#include <keys.h>
#include <game/entities/baseentity.h>

class CCamera : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCamera, CBaseEntity);

public:
					~CCamera();

public:
	void			Spawn();
	virtual void	CameraThink();

	bool			IsTracking();

	virtual void	OnActivated();

	virtual float	GetFOV() const { return m_flFOV; }
	virtual float	GetOrthoHeight() const { return m_flOrthoHeight; }
	virtual float	GetCameraNear() const { return 1; }
	virtual float	GetCameraFar() const { return 10000; }
	virtual bool	ShouldRenderOrthographic() const { return m_bOrtho; }

	virtual void MouseMotion(int x, int y, int dx, int dy) {}
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState) { return false; };
	virtual bool KeyDown(int c) { return false; }
	virtual bool KeyUp(int c) { return false; }
	virtual void TouchMotion(int iFinger, float x, float y, float dx, float dy) { if (iFinger == 0) MouseMotion((int)x, (int)y, (int)dx, (int)dy); };
	virtual bool TouchInput(int iFinger, tinker_mouse_state_t iState, float x, float y) { if (iFinger == 0) return MouseInput(TINKER_KEY_MOUSE_LEFT, iState); else return false; }

protected:
	CEntityHandle<CBaseEntity>	m_hCameraTarget;

	CEntityHandle<CBaseEntity>	m_hTrackStart;
	CEntityHandle<CBaseEntity>	m_hTrackEnd;

	CEntityHandle<CBaseEntity>	m_hTargetStart;
	CEntityHandle<CBaseEntity>	m_hTargetEnd;

	float			m_flFOV;
	float			m_flOrthoHeight;

	bool			m_bOrtho;
};
