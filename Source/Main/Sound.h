#pragma once

class CSound
{
public:
	CSound() : SoundVolumeLevel(5), MusicVolumeLevel(5), MusicPlaying(false) {}

	void Init() {}

	void UpdateSoundVolumeLevel(int volumeLevel) { SoundVolumeLevel = volumeLevel; }
	void UpdateMusicVolumeLevel(int volumeLevel) { MusicVolumeLevel = volumeLevel; }
	void ButtonStopMusic() { MusicPlaying = false; }
	void ButtonPlayMusic() { MusicPlaying = true; }

	int SoundVolumeLevel;
	int MusicVolumeLevel;
	bool MusicPlaying;
};

extern CSound gSound;
