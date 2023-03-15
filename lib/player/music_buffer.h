#pragma once
#include <AL/al.h>
#include "sndfile.h"

class Music_buffer {
public:
	void play();
	//void pause(); 

	void update_buffer_stream();
	ALint get_source();
	Music_buffer(const char* filename);
	~Music_buffer();

	short* mem_buffer;

private:
	ALuint source;
	static const int BUFFER_SAMPLES = 8192;
	static const int NUM_BUFFERS = 4;
	ALuint buffers[NUM_BUFFERS];
	SNDFILE* sound_file;
	SF_INFO sound_file_info;
	ALenum format;

	Music_buffer() = delete;
};


