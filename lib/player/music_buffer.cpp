#include "music_buffer.h"
#include <cstddef>
#include "AL/alext.h"
#include <malloc.h>
#include <iostream>

void Music_buffer::play() {

	alGetError();

	alSourceRewind(source);
	alSourcei(source, AL_BUFFER, 0);

	ALsizei buffer_number;
	for (buffer_number = 0; buffer_number < NUM_BUFFERS; buffer_number++) {

		sf_count_t len = sf_readf_short(sound_file, mem_buffer, BUFFER_SAMPLES);
		if (len < 1) 
			break;

		len *= sound_file_info.channels * (sf_count_t)sizeof(short);
		alBufferData(buffers[buffer_number], format, mem_buffer, (ALsizei)len, sound_file_info.samplerate);
	}

	if (alGetError() != AL_NO_ERROR) {
		std::cout << "Buffering Error";
	}

	alSourceQueueBuffers(source, buffer_number, buffers);
	alSourcePlay(source);

	if (alGetError() != AL_NO_ERROR) {
		std::cout << "starting playback error";
	}
}

//void Music_buffer::pause()

//void Music_buffer::stop()

void Music_buffer::update_buffer_stream() {

	alGetError();

	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

	ALint processed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	if (alGetError() != AL_NO_ERROR) {
			std::cout <<"error checking music source state";
	}

	sf_count_t sound_length;;
	ALuint buffer_id;

	while (processed > 0) {	

		alSourceUnqueueBuffers(source, 1, &buffer_id);
		processed--;

		sound_length = sf_readf_short(sound_file, mem_buffer, BUFFER_SAMPLES);

		if (sound_length > 0) {
			sound_length *= sound_file_info.channels * (sf_count_t)sizeof(short);

			alBufferData(buffer_id, format, mem_buffer, (ALsizei)sound_length, sound_file_info.samplerate);
			alSourceQueueBuffers(source, 1, &buffer_id);
		}

		if (alGetError() != AL_NO_ERROR) {
			std::cout << "buffering music data error";
		}
	}

	if (state != AL_PLAYING && state != AL_PAUSED) {
		
		ALint queued;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

		if (queued == 0)
			return;

		alSourcePlay(source);
		if (alGetError() != AL_NO_ERROR) {
			std::cout << "restarting music playback error";
		}
	}
}

ALint Music_buffer::get_source() {
	return source;
}

Music_buffer::Music_buffer(const char* filename) {
	alGenSources(1, &source);
	alGenBuffers(NUM_BUFFERS, buffers);

	std::size_t frame_size;

	sound_file = sf_open(filename, SFM_READ, &sound_file_info);
	
	if (!sound_file) {
		std::cout << "could not open provided music file";
	}

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


	if (!format) {
		sf_close(sound_file);
		sound_file = NULL;
		std::cout << "Unsupported num of channels from file";
	}

	frame_size = ((size_t)BUFFER_SAMPLES * (size_t)sound_file_info.channels) * sizeof(short);
	mem_buffer = static_cast<short*>(malloc(frame_size));

}

Music_buffer::~Music_buffer() {
	alDeleteSources(1, &source);

	if (sound_file)
		sf_close(sound_file);

	sound_file = nullptr;

	free(mem_buffer);

	alDeleteBuffers(NUM_BUFFERS, buffers);
} 