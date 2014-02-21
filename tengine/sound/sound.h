#ifndef DT_SOUND_H
#define DT_SOUND_H

#include <tmap.h>
#include <tvector.h>

#include <game/entities/baseentity.h>

class CSound
{
public:
							CSound(const tstring& pszFilename);
							~CSound();

public:
	size_t					m_iReferences;

	struct Mix_Chunk*		m_pSound;
	tstring					m_sFilename;
};

class CSoundLibrary
{
public:
							CSoundLibrary();
							~CSoundLibrary();

public:
	static size_t			GetNumSoundsLoaded() { return Get()->m_iSoundsLoaded; };

	static size_t			AddSound(const tstring& pszFilename);
	static size_t			FindSound(const tstring& pszFilename);
	static CSound*			GetSound(size_t i);

public:
	static void				PlaySound(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume = 1.0f, bool bLoop = false);
	static void				StopSound(CBaseEntity* pEntity = NULL, const tstring& pszFilename = "");
	static bool				IsSoundPlaying(CBaseEntity* pEntity, const tstring& pszFilename);

	static void				PlayMusic(const tstring& pszFilename, bool bLoop = false);
	static void				StopMusic();
	static bool				IsMusicPlaying();

	static void				SetSoundVolume(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume);

	static void				SetSoundVolume(float flVolume);
	static void				SetMusicVolume(float flVolume);

	static void				ChannelFinished(int iChannel);
	static void				EntityDeleted(CBaseEntity* pEntity);

	static void				ResetReferenceCounts();
	static void				ClearUnreferenced();

	static CSoundLibrary*	Get() { return s_pSoundLibrary; };

protected:
	class CSoundInstance
	{
	public:
		int iChannel;
		float flVolume;
	};

	tvector<CSound*>		m_apSounds;
	size_t					m_iSoundsLoaded;
	tmap<CBaseEntity*, tmap<tstring, CSoundInstance> >	m_aiActiveSounds;

private:
	static CSoundLibrary*	s_pSoundLibrary;
	static float			s_flMasterVolume;
};

#endif
