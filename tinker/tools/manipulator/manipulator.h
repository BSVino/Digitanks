#pragma once

#include "trs.h"
#include "tstring.h"

#include <glgui/glgui.h>
#include <tinker/keys.h>

class IManipulatorListener
{
public:
	virtual void		ManipulatorUpdated(const tstring& sArguments)=0;
	virtual void		DuplicateMove(const tstring& sArguments)=0;
};

class CManipulatorTool : public glgui::IEventListener
{
public:
	typedef enum
	{
		MT_TRANSLATE,
		MT_ROTATE,
		MT_SCALE,
	} TransformType;

public:
	CManipulatorTool();

public:
	void				Activate(IManipulatorListener* pListener, const TRS& trs=TRS(), const tstring& sArguments="");
	void				Deactivate();
	bool				IsActive() { return m_bActive; }
	bool				IsTransforming();

	void				SetTransfromType(TransformType eTransform);
	TransformType		GetTransfromType() { return m_eTransform; }

	Matrix4x4			GetTransform(bool bRotation = true, bool bScaling = true);
	TRS					GetTRS() { return m_trsTransform; }
	void				SetTRS(const TRS& trs);

	TRS					GetNewTRS();

	class CRenderer*    GetRenderer();

	EVENT_CALLBACK(CManipulatorTool, TranslateMode);
	EVENT_CALLBACK(CManipulatorTool, RotateMode);
	EVENT_CALLBACK(CManipulatorTool, ScaleMode);

	EVENT_CALLBACK(CManipulatorTool, TransformLocal);
	EVENT_CALLBACK(CManipulatorTool, TransformWorld);
	EVENT_CALLBACK(CManipulatorTool, TransformView);

protected:
	bool				m_bActive;
	bool				m_bTransforming;
	TransformType		m_eTransform;
	bool                m_bHasMouse;

	char				m_iLockedAxis;
	float				m_flStartX;
	float				m_flStartY;
	float				m_flOriginalDistance;

	TRS					m_trsTransform;
	Matrix4x4           m_mTransform;

	IManipulatorListener*	m_pListener;
	tstring				m_sListenerArguments;

	glgui::CPictureButton*	m_pTranslateButton;
	glgui::CPictureButton*	m_pRotateButton;
	glgui::CPictureButton*	m_pScaleButton;

	glgui::CMenu*       m_pTransformMenu;

	class IGizmo*       m_pTranslateGizmo;
	class IGizmo*       m_pRotateGizmo;
	class IGizmo*       m_pScaleGizmo;

	class CRenderer*    m_pRenderer;

public:
	static CManipulatorTool*	Get();

	static bool                 MouseInput(int iButton, tinker_mouse_state_t iState, int mx, int my);
	static void                 MouseMoved(int mx, int my);
	static void                 Render(class CRenderer* pRenderer);

protected:
	static CManipulatorTool*	s_pManipulatorTool;
};

inline CManipulatorTool* Manipulator()
{
	return CManipulatorTool::Get();
}
