#include "spotivar-backend.h"
#include "../dummy_scaner/user_files.h"

static std::map<std::string, std::string audio::track_info::*> metadata = {
	{"TITLE", &audio::track_info::title},
	{"ARTIST", &audio::track_info::artist},
	{"ALBUM", &audio::track_info::album},
	{"COMPOSER", &audio::track_info::composer},
	{"YEAR", &audio::track_info::year},
	{"DATE", &audio::track_info::date},
	{"PERFORMER", &audio::track_info::performer},
	{"GENRE", &audio::track_info::genre},
	// {"LENGTH", audio::&mp3_info::length},
	{"COMMENT", &audio::track_info::comment},
	// {"PIC", audio::&mp3_info::pic},
	{"DESCRIPTION", &audio::track_info::description}
};

bool is_in_audio_data(audio::audio_data *track, std::string key_string) {
	//validate!!!
	for (const auto& data_field: metadata) {
		if ((track->info_.*metadata[data_field.first]).find(key_string) != std::string::npos)
			return true;
	}
	return false;
}

template<typename T> 
bool element_is_contained(T elem, std::vector<T> vec) {

	if (vec.size()) 
		return std::find(vec.begin(), vec.end(), elem) != vec.end();
	return false;
}

bool is_in_audio_tags(audio::audio_data *track, std::string key_string) {
	//validate!!!
	for (auto& tag: track->get_tags()) {
		if (tag.find(key_string) != std::string::npos)
			return true;
	}
	return false;
}

search_result *backend::find_tracks_by_prompt(std::string prompt) {
	
	tracklist_t tracklist = session_.user_.tracklist_;

	tracklist_t by_track_metadata = {};
	tracklist_t by_track_tags = {};

	for (const auto& track : tracklist) {
		if (is_in_audio_data(track, prompt)) 
			by_track_metadata.push_back(track);
		if (is_in_audio_tags(track, prompt))
			by_track_tags.push_back(track);
	}

	search_result* result= new search_result(by_track_metadata, by_track_tags);
	return result;
}

directories::directory_content_t *get_directiory_content_by_root(std::string root) {

	directories::directory *dir = new directories::directory(root);
	directory_scaner::read_directory(root.data(), directory_scaner::LINEAR, dir);

	return &dir->get_content();
}
