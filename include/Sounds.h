/////////////////////////////////////////
//
//             Liero Xtreme
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Sounds header file
// Created 29/7/02
// Jason Boettcher


#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include "CViewport.h"
#include "CMap.h"

// we are using SDL_mixer at the moment
#include <SDL/SDL_mixer.h>

#define MUSIC_REPEAT_INFINITE -1

#define SND_CMD MUS_CMD
#define SND_WAV MUS_WAV
#define SND_MOD MUS_MOD
#define SND_MID MUS_MID
#define SND_OGG MUS_OGG
#define SND_MP3 MUS_MP3

// this typedef can be replaced if another sound system is wanted
// also, all *Sound* functions need to be recoded then
// for using this, handle with pointers of it
struct SoundSample {
	Mix_Chunk* sample;
	int maxsimulplays;
// TODO: and other stuff
};

// Music
struct SoundMusic {
	Mix_Music *sndMusic;
};

// General sounds
typedef struct {
	SoundSample*		smpClick;
	SoundSample*		smpChat;
} sfxgen_t;


// Game sounds
typedef struct {
	SoundSample*		smpNinja;
	SoundSample*		smpPickup;
	SoundSample*		smpBump;
	SoundSample*		smpDeath[3];
} sfxgame_t;


// Routines
bool	InitSoundSystem(int rate, int channels, int buffers);
bool	StartSoundSystem();
bool	StopSoundSystem();
bool	SetSoundVolume(int vol);
bool	QuitSoundSystem();
SoundSample* LoadSoundSample(char* filename, int maxsimulplays);
bool	FreeSoundSample(SoundSample* sample);
bool	PlaySoundSample(SoundSample* sample);
// TODO: the music part, for example:
// int	LoadSoundMusic();
// int	PlaySoundMusic();
// etc.

int		LoadSounds(void);
void	ShutdownSounds(void);
void	StartSound(SoundSample* smp, CVec pos, int local, int volume, CWorm *me);

extern float fCurSongStart;
extern float fTimePaused;

// Music
SoundMusic		*LoadMusic(const char *file);
void			FreeMusic(SoundMusic *music);
void			PlayMusic(SoundMusic *music, int number_of_repeats=1);
inline void		PauseMusic(void) {Mix_PauseMusic(); fTimePaused = GetMilliSeconds();}
inline void		ResumeMusic(void) {Mix_ResumeMusic();fCurSongStart += GetMilliSeconds()-fTimePaused; fTimePaused = 0;}
inline void		RewindMusic(void) {Mix_RewindMusic();fCurSongStart = GetMilliSeconds();fTimePaused = 0;}
inline void		SetMusicPosition(unsigned int pos)  {Mix_RewindMusic(); Mix_SetMusicPosition(pos); }
inline void		StopMusic(void) {Mix_HaltMusic(); fCurSongStart = 0; fTimePaused = 0; }
inline bool		PlayingMusic(void) {return Mix_PlayingMusic() != 0; }
inline bool		PausedMusic(void) {return Mix_PausedMusic() != 0; }
inline int		GetMusicType(SoundMusic *music = NULL) {if (music) {return Mix_GetMusicType(music->sndMusic);} else {return Mix_GetMusicType(NULL);} }
float			GetCurrentMusicTime(void);

void		SetMusicVolume(byte vol);





// Globals
extern	sfxgame_t	sfxGame;
extern	sfxgen_t	sfxGeneral;





#endif  //  __SOUNDS_H__
