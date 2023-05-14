#include "session.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

void session::print_session_info() {

	std::cout << "\n--------SESSION_INFO--------\n";

	std::cout << "USERNAME :\t" << user_.username_ << std::endl;
	// std::cout << "USER TRACKLIST :\n";
	// for (auto& track: user_.tracklist_) 
	// 	track->print_track_info();
	std::cout << "DIRECTORIES FOR SCANNING :\n";
	for (const auto& dir : directories_to_scan_)
		std::cout << "\t\t" << dir << std::endl;
}

session::session() {

	boost::filesystem::path config_path = "/home/varvara/.config/merth";
	
	if (!boost::filesystem::exists(config_path)) {

		std::cout << "~/.config/merth not found, starting a first session";
		boost::filesystem::create_directory(config_path);
		is_first = true;

	} else {

		std::cout << "!!! starting a new session !!! \n";
		
		Json::Value previous_session;
		std::ifstream session_json_file("/home/varvara/.config/merth/last_session.json", std::ifstream::binary);
		session_json_file >> previous_session;
		// std::cout << previous_session["directories_to_scan"][0];

		user_.username_ = previous_session["user"]["username"].asString();

		const Json::Value user_tracklist = previous_session["user"]["tracklist"];
		for (int i = 0; i < user_tracklist.size(); i++)	
			user_.tracklist_.push_back(new audio::audio_data(user_tracklist[i]));
		
		const Json::Value directories_for_scan = previous_session["directories_to_scan"];
		for (int i = 0; i < directories_for_scan.size(); i++)	
			directories_to_scan_.push_back(directories_for_scan[i].asString());
	}
}


void ask_name() {
	//something...
	
};

void ask_derictory() {};

void session::start() {
	
	if (is_first) {
		ask_name();
		ask_derictory();
	}
}
