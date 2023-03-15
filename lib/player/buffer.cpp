#include "buffer.h"

#include "sndfile.h"
#include <iostream>
#include <bits/stdc++.h>
#include <inttypes.h>
#include "AL/alext.h"

Buffer* Buffer::get_buffer() {
    static Buffer* buffer = new Buffer();
    return buffer;
}


// static enum 

ALuint Buffer::add_sound(const char* filename)
{
	SNDFILE* sound_file;
	SF_INFO sound_file_info;

	short* memory_buffer;
	sf_count_t num_of_frames;
	ALsizei num_of_bytes;

	sound_file = sf_open(filename, SFM_READ, &sound_file_info);

    if (!sound_file) {
        std::cerr << "failed to open file"; 
        return 0;  // <-------------enum ````````
    }

	if (sound_file_info.frames < 1 || sound_file_info.frames > (sf_count_t)(INT_MAX / sizeof(short)) / sound_file_info.channels) {
		std::cerr << "Bad num of samples in "<< filename << "--> " << sound_file_info.frames;
		sf_close(sound_file);
		return 0; // <-----------------enum
	}


	ALenum format = AL_NONE;

	if (sound_file_info.channels == 1)
		format = AL_FORMAT_MONO16;

	else if (sound_file_info.channels == 2)
		format = AL_FORMAT_STEREO16;

	else if (sound_file_info.channels == 3) {
		if (sf_command(sound_file, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT2D_16;
	}

	else if (sound_file_info.channels == 4) {
		if (sf_command(sound_file, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT3D_16;
	}


	if (!format)
	{
		std::cerr << "Unsupported channel count: (" << sound_file_info.channels << ")\n";
		sf_close(sound_file);
		return 0; //<----------- enum
	}


	memory_buffer = static_cast<short*>(malloc((size_t)(sound_file_info.frames * sound_file_info.channels) * sizeof(short)));

	num_of_frames = sf_readf_short(sound_file, memory_buffer, sound_file_info.frames);

	if (num_of_frames <= 0) {
		free(memory_buffer);
		sf_close(sound_file);

		std::cerr << "failed to read samples in " << filename << "\n";
		return 0; //<---------enum
	}

	num_of_bytes = (ALsizei)(num_of_frames * sound_file_info.channels) * (ALsizei)sizeof(short);


	ALuint buffer = 0;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, memory_buffer, num_of_bytes, sound_file_info.samplerate);

	free(memory_buffer);
	sf_close(sound_file);


	ALenum err = alGetError();
	if (err != AL_NO_ERROR)
	{ 
		std::cerr << "OpenAL error((((((: " << alGetString(err);
		if (buffer && alIsBuffer(buffer))
			alDeleteBuffers(1, &buffer);
		return 0;
	}

	sound_buffers.push_back(buffer);  

	return buffer;
}


bool Buffer::remove_sound(const ALuint& buffer) {
	auto it = sound_buffers.begin();

	while (it != sound_buffers.end()) {
		if (*it == buffer) {

			alDeleteBuffers(1, &*it);
			it = sound_buffers.erase(it);

			return true;
		}
		else {
			++it;
		}
	}
	return false;  
}


Buffer::Buffer() {
	sound_buffers.clear();
}

Buffer::~Buffer() {
	alDeleteBuffers(sound_buffers.size(), sound_buffers.data());
	sound_buffers.clear();
}
