#include "sound.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include <strutils.h>

CSoundLibrary* CSoundLibrary::s_pSoundLibrary = NULL;
float CSoundLibrary::s_flMasterVolume = 1;
static CSoundLibrary g_SoundLibrary = CSoundLibrary();

CSoundLibrary::CSoundLibrary()
{
	s_pSoundLibrary = this;
	m_iSoundsLoaded = 0;

	SDL_Init(SDL_INIT_AUDIO);

	if(Mix_OpenAudio(22050, AUDIO_S16, 2, 4096))
		SDL_Quit();

	Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
}

CSoundLibrary::~CSoundLibrary()
{
	for (size_t i = 0; i < m_apSounds.size(); i++)
		delete m_apSounds[i];

	Mix_CloseAudio();

	s_pSoundLibrary = NULL;
}

size_t CSoundLibrary::AddSound(const tstring& pszFilename)
{
	size_t iSound = FindSound(pszFilename);
	if (iSound != ~0)
	{
		Get()->m_apSounds[iSound]->m_iReferences++;
		return iSound;
	}

	size_t iLocation = ~0;
	for (size_t i = 0; i < Get()->m_apSounds.size(); i++)
	{
		if (!Get()->m_apSounds[i])
		{
			iLocation = i;
			break;
		}
	}

	if (iLocation == ~0)
	{
		iLocation = Get()->m_apSounds.size();
		Get()->m_apSounds.push_back();
	}

	Get()->m_iSoundsLoaded++;

	Get()->m_apSounds[iLocation] = new CSound(pszFilename);

	Get()->m_apSounds[iSound]->m_iReferences++;
	return iLocation;
}

CSound* CSoundLibrary::GetSound(size_t i)
{
	if (i >= Get()->m_apSounds.size())
		return NULL;

	return Get()->m_apSounds[i];
}

size_t CSoundLibrary::FindSound(const tstring& pszFilename)
{
	for (size_t i = 0; i < Get()->m_apSounds.size(); i++)
	{
		if (!Get()->m_apSounds[i])
			continue;

		if (Get()->m_apSounds[i]->m_sFilename == pszFilename)
			return i;
	}

	return ~0;
}

void CSoundLibrary::PlaySound(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume, bool bLoop)
{
	if (Get()->m_aiActiveSounds.find(pEntity) != Get()->m_aiActiveSounds.end())
	{
		if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) != Get()->m_aiActiveSounds[pEntity].end())
			StopSound(pEntity, pszFilename);
	}

	size_t iSound = Get()->FindSound(pszFilename);

	if (iSound >= Get()->m_apSounds.size())
		return;

	CSound* pSound = Get()->m_apSounds[iSound];

	if (!pSound)
		return;

	if( pSound->m_pSound == NULL )
		return;

	int iChannel = Mix_PlayChannel(-1, pSound->m_pSound, bLoop?-1:0);
	if (iChannel < 0)
		return;

	Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel = iChannel;
	Get()->m_aiActiveSounds[pEntity][pszFilename].flVolume = flVolume;
	Mix_Volume(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel, (int)(flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
}

void CSoundLibrary::StopSound(CBaseEntity* pEntity, const tstring& pszFilename)
{
	if (!pEntity)
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		tmap<CBaseEntity*, tmap<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin();

		while (it != Get()->m_aiActiveSounds.end())
		{
			tmap<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[(*it).first].begin();
			while (it2 != Get()->m_aiActiveSounds[(*it).first].end())
			{
				Mix_HaltChannel(Get()->m_aiActiveSounds[(*it).first][(*it2).first].iChannel);
				it2++;
			}

			it++;
		}

		Get()->m_aiActiveSounds.clear();

		Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
		return;
	}

	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return;

	if (pszFilename != "")
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		tmap<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][(*it2).first].iChannel);
			it2++;
		}

		Get()->m_aiActiveSounds[pEntity].clear();

		Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
		return;
	}

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel);
}

bool CSoundLibrary::IsSoundPlaying(CBaseEntity* pEntity, const tstring& pszFilename)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return false;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return false;

	return true;
}

Mix_Music *g_pMusic = NULL;
void CSoundLibrary::PlayMusic(const tstring& sFilename, bool bLoop)
{
	if (g_pMusic)
		StopMusic();

	g_pMusic = Mix_LoadMUS(sFilename.c_str());

	Mix_PlayMusic(g_pMusic, bLoop?-1:0);
}

void CSoundLibrary::StopMusic()
{
	if (!g_pMusic)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic(g_pMusic);	// Because music wants to be free!

	g_pMusic = NULL;
}

bool CSoundLibrary::IsMusicPlaying()
{
	return !!Mix_PlayingMusic();
}

void CSoundLibrary::SetSoundVolume(CBaseEntity* pEntity, const tstring& pszFilename, float flVolume)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Get()->m_aiActiveSounds[pEntity][pszFilename].flVolume = flVolume;
	Mix_Volume(Get()->m_aiActiveSounds[pEntity][pszFilename].iChannel, (int)(flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
}

void CSoundLibrary::SetSoundVolume(float flVolume)
{
	s_flMasterVolume = flVolume;
	for (tmap<CBaseEntity*, tmap<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin(); it != Get()->m_aiActiveSounds.end(); it++)
	{
		for (tmap<tstring, CSoundInstance>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
			Mix_Volume(it2->second.iChannel, (int)(it2->second.flVolume*s_flMasterVolume*MIX_MAX_VOLUME));
	}
}

void CSoundLibrary::SetMusicVolume(float flVolume)
{
	Mix_VolumeMusic((int)(flVolume*128));
}

void CSoundLibrary::ChannelFinished(int iChannel)
{
	tmap<CBaseEntity*, tmap<tstring, CSoundInstance> >::iterator it = Get()->m_aiActiveSounds.begin();

	while (it != Get()->m_aiActiveSounds.end())
	{
		CBaseEntity* pEntity = (*it).first;
		tmap<tstring, CSoundInstance>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			tstring pszFilename = (*it2).first;
			if (iChannel == Get()->m_aiActiveSounds[pEntity][(*it2).first].iChannel)
			{
				Get()->m_aiActiveSounds[pEntity].erase(Get()->m_aiActiveSounds[pEntity].find(pszFilename));
				if (Get()->m_aiActiveSounds[pEntity].size() == 0)
					Get()->m_aiActiveSounds.erase(Get()->m_aiActiveSounds.find(pEntity));
				return;
			}

			it2++;
		}

		it++;
	}
}

void CSoundLibrary::EntityDeleted(CBaseEntity* pEntity)
{
	// Remove from the active sound list. Let the sounds finish what they're playing.
	if (Get()->m_aiActiveSounds.find(pEntity) != Get()->m_aiActiveSounds.end())
		Get()->m_aiActiveSounds.erase(Get()->m_aiActiveSounds.find(pEntity));
}

void CSoundLibrary::ResetReferenceCounts()
{
	for (size_t i = 0; i < Get()->m_apSounds.size(); i++)
	{
		if (!Get()->m_apSounds[i])
			continue;

		Get()->m_apSounds[i]->m_iReferences = 0;
	}
}

void CSoundLibrary::ClearUnreferenced()
{
	for (size_t i = 0; i < Get()->m_apSounds.size(); i++)
	{
		if (!Get()->m_apSounds[i])
			continue;

		if (!Get()->m_apSounds[i]->m_iReferences)
		{
			Get()->m_iSoundsLoaded--;
			delete Get()->m_apSounds[i];
			Get()->m_apSounds[i] = nullptr;
		}
	}
}

CSound::CSound(const tstring& sFilename)
{
	m_iReferences = 0;

	SDL_RWops* pRW = SDL_RWFromFile(sFilename.c_str(), "rb");

	m_pSound = Mix_LoadWAV_RW(pRW, 1);

	m_sFilename = sFilename;
}

CSound::~CSound()
{
	if( m_pSound != NULL )
	{
		Mix_FreeChunk( m_pSound );
	}
}
