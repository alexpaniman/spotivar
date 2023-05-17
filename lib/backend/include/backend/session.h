#pragma once

#include <iostream>
#include <vector>

#include "../audio_data_decoder/audio_data.h"

using tracklist_t = std::vector<audio::audio_data*>;
using playlists_y = std::vector<audio::audio_data*>;

class user {
public:
	std::string username_ = "pc_destroyer1337";
	tracklist_t tracklist_ = {};
	
	
};

class session {
public:
	session();

	void print_session_info();

	void start();
	void read_tracklist_from_user_directories();
	void read_tracklist_from_json();
	void update_tracklist();
	void get_tracklist();

	void add_directory_to_scan();
	void remove_directory_to_scan();
	void add_file_to_scan();

	std::vector<std::string> directories_to_scan_;
	user user_;
	bool is_first = false;
};

class track: public audio::audio_data {
public:
	void add_to_playlist();
	void remove_from_playlist(tracklist_t);
	void remove_from_tracklist(user*);
	void add_tag();
	void delete_tag();
	void show_artist();
	void show_album();
};

