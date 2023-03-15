#include "device.h"
#include "music_buffer.h"
#include "source.h"
#include <iostream>

#include "FLAC/format.h"
#include "FLAC/metadata.h"
#include "FLAC_track_info.h"


int main() {

    std::cout << "start!\n";
    Device* active_device = Device::get_device();

    Source my_speaker;

	Music_buffer my_music("/home/varvara/mipt/player/testt.wav");

	std::cout << "playing music...\n";
	my_music.play();

	ALint state = AL_PLAYING;
	std::cout << "playing sound\n";
	while (state == AL_PLAYING && alGetError() == AL_NO_ERROR) {
		my_music.update_buffer_stream();
		alGetSourcei(my_music.get_source(), AL_SOURCE_STATE, &state);
	}

	std::cout << "got here\n";

    std::cout << "end!\n";
	// FLAC_track_info file_info;
	// FLAC_read_track_info("/home/varvara/mipt/player/flac_tests/aesop.flac", &file_info);

	// FLAC__StreamMetadata* tags;

	// std::cout << FLAC__metadata_get_tags("/home/varvara/mipt/player/flac_tests/microorganism.flac", &tags);

    return 0;
} 