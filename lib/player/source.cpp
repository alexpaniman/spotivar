#include "source.h"
#include <iostream>

Source::Source() {
	alGenSources(1, &source);
	alSourcef(source, AL_PITCH, pitch);
	alSourcef(source, AL_GAIN, gain);
	alSource3f(source, AL_POSITION, position[0], position[1], position[2]);
	alSource3f(source, AL_VELOCITY, velocity[0], velocity[1], velocity[2]);
	alSourcei(source, AL_LOOPING, loop_sound);
	alSourcei(source, AL_BUFFER, buffer);
}

Source::~Source() {
	alDeleteSources(1, &source);
}

void Source::play(const ALuint buffer_to_play) {
	if (buffer_to_play != buffer) {
			buffer = buffer_to_play;
			alSourcei(source, AL_BUFFER, (ALint)buffer);
	}
	alSourcePlay(source);

	ALint state = AL_PLAYING;
	std::cout << "playing!!!\n";

	while (state == AL_PLAYING && alGetError() == AL_NO_ERROR) {
		std::cout << "currently playing!!!\n";
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	}

	std::cout << "done\n";
}