#include "directory-scanner/user_files.h"
#include <iostream>
#include <boost/range/iterator_range.hpp>
#include <string>
#include "audio-data-decoder/audio_data.h"

using namespace directories;
using namespace boost::filesystem;

static const int HEADER_SIZE = 10;

std::string get_os_name() {
    #ifdef _WIN32
    	return "Windows 32-bit";
    #elif _WIN64
    	return "Windows 64-bit";
    #elif __APPLE__ || __MACH__
    	return "Mac OSX";
    #elif __linux__
    	return "Linux";
    #elif __FreeBSD__
    	return "FreeBSD";
    #elif __unix || __unix__
    	return "Unix";
    #else
    	return "Other";
    #endif
}

enum audio_extension_ : char {OGG = 0, FLAC, MP3, NOT_AN_AUDIO};

audio_extension_ check_audio_extension(std::string file_path) {

	FILE* file = fopen(file_path.c_str(), "r");
	if (!file) return NOT_AN_AUDIO;

	std::string buffer(4, '\0');

	fread(&buffer[0], 1, HEADER_SIZE, file);
	fclose(file);

	if (buffer == "fLaC") return FLAC;
	if (buffer.substr(0, 3) == "ID3") return MP3;
	if (buffer == "OggS") return OGG;

	return NOT_AN_AUDIO;
}


directory *read_directory(const char *path_to_root_directory, directory_scaner::reading_mod mod, directory *dir) {

	// std::cout <<  mod << std::endl;
	path root_path{path_to_root_directory};

	audio::tracklist_t tracks = {};
	directory_iterator it{root_path};
	
	if (is_directory(root_path)) {

		for (auto& entry : boost::make_iterator_range(directory_iterator(root_path), {})) {

			if (is_regular_file(entry)) {

				audio_extension_ extension = check_audio_extension(entry.path().string());

				bool is_audio = (extension == MP3) || (extension == FLAC); // || (extension == "OggS");
				if (is_audio)
					dir->content_.push_back(new directory_content_obj(entry.path().string(), entry.path().filename().string(), ENTRY));
			}

			else if (is_directory(entry)) {

				dir->content_.push_back(new directory_content_obj(entry.path().string(), entry.path().filename().string(), FOLDER));

				if (mod == directory_scaner::RECURSIVE) 
					directory_scaner::read_directory(entry.path().c_str(), directory_scaner::RECURSIVE, dir);
			}
		}
	}

	std::cout << dir->content_.size() << std::endl;

	return dir;
}
