#include "sound.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>

CSoundLibrary* CSoundLibrary::s_pSoundLibrary = NULL;
static CSoundLibrary g_SoundLibrary = CSoundLibrary();

CSoundLibrary::CSoundLibrary()
{
	s_pSoundLibrary = this;

	SDL_Init(SDL_INIT_AUDIO);

	if(Mix_OpenAudio(22050, AUDIO_S16, 2, 4096))
		SDL_Quit();

	Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
}

CSoundLibrary::~CSoundLibrary()
{
	for (size_t i = 0; i < m_apSounds.size(); i++)
	{
		delete m_apSounds[i];
	}

	Mix_CloseAudio();

	s_pSoundLibrary = NULL;
}

size_t CSoundLibrary::AddSound(const char* pszFilename)
{
	size_t iSound = FindSound(pszFilename);
	if (iSound != ~0)
		return iSound;

	m_apSounds.push_back(new CSound(pszFilename));

	iSound = m_apSounds.size()-1;
	return iSound;
}

CSound* CSoundLibrary::GetSound(size_t i)
{
	if (i >= m_apSounds.size())
		return NULL;

	return m_apSounds[i];
}

size_t CSoundLibrary::FindSound(const char* pszFilename)
{
	for (size_t i = 0; i < m_apSounds.size(); i++)
	{
		if (strcmp(m_apSounds[i]->m_sFilename.c_str(), pszFilename) == 0)
			return i;
	}

	return ~0;
}

void CSoundLibrary::PlaySound(CBaseEntity* pEntity, const char* pszFilename)
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

	if( pSound->m_pSound == NULL )
		return;

	int iChannel = Mix_PlayChannel(-1, pSound->m_pSound, 0);
	if (iChannel < 0)
		return;

	Get()->m_aiActiveSounds[pEntity][pszFilename] = iChannel;
}

void CSoundLibrary::StopSound(CBaseEntity* pEntity, const char* pszFilename)
{
	if (!pEntity)
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		std::map<CBaseEntity*, std::map<const char*, int> >::iterator it = Get()->m_aiActiveSounds.begin();

		while (it != Get()->m_aiActiveSounds.end())
		{
			std::map<const char*, int>::iterator it2 = Get()->m_aiActiveSounds[(*it).first].begin();
			while (it2 != Get()->m_aiActiveSounds[(*it).first].end())
			{
				Mix_HaltChannel(Get()->m_aiActiveSounds[(*it).first][(*it2).first]);
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

	if (!pszFilename)
	{
		// Un-register the channel finish callback for the time being because we clear the active sound list by ourselves.
		Mix_ChannelFinished(NULL);

		std::map<const char*, int>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][(*it2).first]);
			it2++;
		}

		Get()->m_aiActiveSounds[pEntity].clear();

		Mix_ChannelFinished(&CSoundLibrary::ChannelFinished);
		return;
	}

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Mix_HaltChannel(Get()->m_aiActiveSounds[pEntity][pszFilename]);
}

bool CSoundLibrary::IsSoundPlaying(CBaseEntity* pEntity, const char* pszFilename)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return false;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return false;

	return true;
}

void CSoundLibrary::SetSoundVolume(CBaseEntity* pEntity, const char* pszFilename, float flVolume)
{
	if (Get()->m_aiActiveSounds.find(pEntity) == Get()->m_aiActiveSounds.end())
		return;

	if (Get()->m_aiActiveSounds[pEntity].find(pszFilename) == Get()->m_aiActiveSounds[pEntity].end())
		return;

	Mix_Volume(Get()->m_aiActiveSounds[pEntity][pszFilename], (int)(flVolume*MIX_MAX_VOLUME));
}

void CSoundLibrary::ChannelFinished(int iChannel)
{
	std::map<CBaseEntity*, std::map<const char*, int> >::iterator it = Get()->m_aiActiveSounds.begin();

	while (it != Get()->m_aiActiveSounds.end())
	{
		CBaseEntity* pEntity = (*it).first;
		std::map<const char*, int>::iterator it2 = Get()->m_aiActiveSounds[pEntity].begin();
		while (it2 != Get()->m_aiActiveSounds[pEntity].end())
		{
			const char* pszFilename = (*it2).first;
			if (iChannel == Get()->m_aiActiveSounds[pEntity][(*it2).first])
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

CSound::CSound(const char* pszFilename)
{
	SDL_RWops* pRW = SDL_RWFromFile(pszFilename, "rb");

	m_pSound = Mix_LoadWAV_RW(pRW, 1);

	m_sFilename = pszFilename;
}

CSound::~CSound()
{
	if( m_pSound != NULL )
	{
		Mix_FreeChunk( m_pSound );
	}
}
