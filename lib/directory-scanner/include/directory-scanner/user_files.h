#ifndef USER_FILES_READER_H
#define USER_FILES_READER_H

#include <boost/filesystem.hpp>

#include "audio-data-decoder/audio_data.h"
#include "directory-scanner/directories.h"

namespace directory_scaner {
	enum reading_mod : char { RECURSIVE = 0, LINEAR };

	static directories::directory *directory_content = new directories::directory();
	directories::directory *read_directory(const char *path_to_root_directory = "/", reading_mod mod = LINEAR, directories::directory *dir = directory_content);
}

#endif
