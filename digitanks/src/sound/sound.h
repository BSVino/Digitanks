#ifndef DT_SOUND_H
#define DT_SOUND_H

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

#include <game/baseentity.h>

class CSound
{
public:
							CSound(const tstring& pszFilename);
							~CSound();

public:
	struct Mix_Chunk*		m_pSound;
	tstring			m_sFilename;
};

class CSoundLibrary
{
public:
							CSoundLibrary();
							~CSoundLibrary();

public:
	static size_t			GetNumSounds() { return Get()->m_apSounds.size(); };

	size_t					AddSound(const tstring& pszFilename);
	size_t					FindSound(const tstring& pszFilename);
	CSound*					GetSound(size_t i);

public:
	static void				PlaySound(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume = 1.0f, bool bLoop = false);
	static void				StopSound(CBaseEntity* pEntity = NULL, const tstring& pszFilename = _T(""));
	static bool				IsSoundPlaying(CBaseEntity* pEntity, const tstring& pszFilename);

	static void				PlayMusic(const tstring& pszFilename, bool bLoop = false);
	static void				StopMusic();
	static bool				IsMusicPlaying();

	static void				SetSoundVolume(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume);

	static void				SetSoundVolume(float flVolume);
	static void				SetMusicVolume(float flVolume);

	static void				ChannelFinished(int iChannel);
	static void				EntityDeleted(CBaseEntity* pEntity);

	static CSoundLibrary*	Get() { return s_pSoundLibrary; };

protected:
	class CSoundInstance
	{
	public:
		int iChannel;
		float flVolume;
	};

	eastl::vector<CSound*>	m_apSounds;
	eastl::map<CBaseEntity*, eastl::map<tstring, CSoundInstance> >	m_aiActiveSounds;

private:
	static CSoundLibrary*	s_pSoundLibrary;
	static float			s_flMasterVolume;
};

#endif
