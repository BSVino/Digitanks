#ifndef DT_SOUND_H
#define DT_SOUND_H

#include <vector>
#include <map>

#include <game/baseentity.h>

class CSound
{
public:
							CSound(const char* pszFilename);
							~CSound();

public:
	struct Mix_Chunk*		m_pSound;
	std::string				m_sFilename;
};

class CSoundLibrary
{
public:
							CSoundLibrary();
							~CSoundLibrary();

public:
	size_t					GetNumSounds() { return m_apSounds.size(); };

	size_t					AddSound(const char* pszFilename);
	size_t					FindSound(const char* pszFilename);
	CSound*					GetSound(size_t i);

public:
	static void				PlaySound(CBaseEntity* pEntity, const char* pszFilename);
	static void				StopSound(CBaseEntity* pEntity = NULL, const char* pszFilename = NULL);
	static bool				IsSoundPlaying(CBaseEntity* pEntity, const char* pszFilename);

	static void				PlayMusic(const char* pszFilename, bool bLoop = false);
	static void				StopMusic();
	static bool				IsMusicPlaying();

	static void				SetSoundVolume(CBaseEntity* pEntity, const char* pszFilename, float flVolume);

	static void				SetSoundVolume(float flVolume);
	static void				SetMusicVolume(float flVolume);

	static void				ChannelFinished(int iChannel);
	static void				EntityDeleted(CBaseEntity* pEntity);

	static CSoundLibrary*	Get() { return s_pSoundLibrary; };

protected:
	std::vector<CSound*>	m_apSounds;
	std::map<CBaseEntity*, std::map<const char*, int> >	m_aiActiveSounds;

private:
	static CSoundLibrary*	s_pSoundLibrary;
};

#endif