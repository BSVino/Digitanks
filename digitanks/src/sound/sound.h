#ifndef DT_SOUND_H
#define DT_SOUND_H

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

#include <game/baseentity.h>

class CSound
{
public:
							CSound(const eastl::string16& pszFilename);
							~CSound();

public:
	struct Mix_Chunk*		m_pSound;
	eastl::string16			m_sFilename;
};

class CSoundLibrary
{
public:
							CSoundLibrary();
							~CSoundLibrary();

public:
	size_t					GetNumSounds() { return m_apSounds.size(); };

	size_t					AddSound(const eastl::string16& pszFilename);
	size_t					FindSound(const eastl::string16& pszFilename);
	CSound*					GetSound(size_t i);

public:
	static void				PlaySound(CBaseEntity* pEntity, const eastl::string16& pszFilename);
	static void				StopSound(CBaseEntity* pEntity = NULL, const eastl::string16& pszFilename = L"");
	static bool				IsSoundPlaying(CBaseEntity* pEntity, const eastl::string16& pszFilename);

	static void				PlayMusic(const eastl::string16& pszFilename, bool bLoop = false);
	static void				StopMusic();
	static bool				IsMusicPlaying();

	static void				SetSoundVolume(CBaseEntity* pEntity, const eastl::string16& pszFilename, float flVolume);

	static void				SetSoundVolume(float flVolume);
	static void				SetMusicVolume(float flVolume);

	static void				ChannelFinished(int iChannel);
	static void				EntityDeleted(CBaseEntity* pEntity);

	static CSoundLibrary*	Get() { return s_pSoundLibrary; };

protected:
	eastl::vector<CSound*>	m_apSounds;
	eastl::map<CBaseEntity*, eastl::map<eastl::string16, int> >	m_aiActiveSounds;

private:
	static CSoundLibrary*	s_pSoundLibrary;
};

#endif