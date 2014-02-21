#include "camera.h"

#include <geometry.h>

#include <game/cameramanager.h>

REGISTER_ENTITY(CCamera);

NETVAR_TABLE_BEGIN(CCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN_EDITOR(CCamera);
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hCameraTarget, "CameraTarget");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTrackStart, "TrackStart");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTrackEnd, "TrackEnd");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTargetStart, "TargetStart");
	SAVEDATA_DEFINE_HANDLE_ENTITY(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseEntity>, m_hTargetEnd, "TargetEnd");
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, float, m_flFOV, "FOV", 45);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, float, m_flOrthoHeight, "OrthoHeight", 10);
	SAVEDATA_DEFINE_HANDLE_DEFAULT(CSaveData::DATA_COPYTYPE, bool, m_bOrtho, "Ortho", false);
	SAVEDATA_EDITOR_VARIABLE("Ortho");
	SAVEDATA_EDITOR_VARIABLE("CameraTarget");
	SAVEDATA_EDITOR_VARIABLE("TrackStart");
	SAVEDATA_EDITOR_VARIABLE("TrackEnd");
	SAVEDATA_EDITOR_VARIABLE("TargetStart");
	SAVEDATA_EDITOR_VARIABLE("TargetEnd");
	SAVEDATA_EDITOR_VARIABLE("FOV");
	SAVEDATA_EDITOR_VARIABLE("OrthoHeight");
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCamera);
INPUTS_TABLE_END();

CCamera::~CCamera()
{
	if (CameraManager())
		CameraManager()->RemoveCamera(this);
}

void CCamera::Spawn()
{
	TAssert(CameraManager());
	CameraManager()->AddCamera(this);
}

void CCamera::CameraThink()
{
	if (IsTracking())
	{
		Vector vecTargetPosition = m_hCameraTarget->GetGlobalOrigin();

		Vector vecTargetStart = m_hTargetStart->GetGlobalOrigin();
		Vector vecTargetEnd = m_hTargetEnd->GetGlobalOrigin();

		Vector vecClosestPoint;
		DistanceToLineSegment(vecTargetPosition, vecTargetStart, vecTargetEnd, &vecClosestPoint);

		float flTargetLength = (vecTargetEnd-vecTargetStart).Length();
		TAssert(flTargetLength > 0);
		if (flTargetLength > 0)
		{
			float flLerp = (vecClosestPoint-vecTargetStart).Length()/flTargetLength;

			Vector vecTrackStart = m_hTrackStart->GetGlobalOrigin();
			Vector vecTrackEnd = m_hTrackEnd->GetGlobalOrigin();

			SetGlobalOrigin(vecTrackStart + (vecTrackEnd-vecTrackStart)*flLerp);

			Vector vecTrackStartDirection = (vecTargetStart-vecTrackStart).Normalized();
			Vector vecTrackEndDirection = (vecTargetEnd-vecTrackEnd).Normalized();

			SetGlobalAngles(VectorAngles(vecTrackStartDirection + (vecTrackEndDirection-vecTrackStartDirection)*flLerp));
		}
	}
}

bool CCamera::IsTracking()
{
	if (m_hCameraTarget && m_hTrackStart && m_hTrackEnd && m_hTargetStart && m_hTargetEnd)
		return true;

	return false;
}

void CCamera::OnActivated()
{
	bool bSnap = false;
	if (s_sCurrentInput.length())
	{
		tvector<tstring> asArgs;
		tstrtok(s_sCurrentInput, asArgs);

		for (size_t i = 1; i < asArgs.size(); i++)
		{
			if (asArgs[i].comparei("snap") == 0)
			{
				bSnap = true;
				break;
			}
		}
	}

	CameraManager()->SetActiveCamera(this, bSnap);
}
