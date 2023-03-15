#pragma once

#include <AL/al.h>

class Source {
public:
	Source();
	~Source();

	void play(const ALuint buffer_to_play);

private:
	ALuint source;
	float pitch = 1.f;
	float gain = 1.f;
	float position[3] = { 0,0,0 };
	float velocity[3] = { 0,0,0 };
	bool loop_sound = false;
	ALuint buffer = 0;
};
