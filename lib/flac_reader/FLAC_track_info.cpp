#include "FLAC_track_info.h"
#include <vector>
#include <algorithm>
#include <map>

static std::map<std::string, std::string FLAC_track_info::*> metadata = {
    {"TITLE",  &FLAC_track_info::title},
    {"ARTIST", &FLAC_track_info::artist},
    {"ALBUM",  &FLAC_track_info::album}, 
    {"COMMENT", &FLAC_track_info::comment},
    {"DATE", &FLAC_track_info::date},
    {"GENRE", &FLAC_track_info::genre},
    {"ALBUM_ARTIST", &FLAC_track_info::album_artist},
};


bool FLAC_extract_fields_info(std::vector<std::string> *data_fields, std::map <std::string, std::string> *data_fields_info) {

	const char separator = '=';
	int separator_position;

	for (std::string data_field : *data_fields ) {

		separator_position = data_field.find(separator);
		(*data_fields_info)[data_field.substr(0, separator_position)] = data_field.substr(separator_position+1, data_field.length()-1);
		// std::cout << (*data_fields_info)[data_field.substr(0, separator_position)] << std::endl;
	}
	
	return true; //<------- !!!!!
}


static const std::vector<std::string> fields = {"TITLE", "ARTIST", "ALBUM", "COMMENT", "DATE", "GENRE", "ALBUM_ARTIST"};


void FLAC_fill_track_info(FLAC_track_info *track_info, std::map <std::string, std::string> *data_fields_info) {

	for (auto& field_data : *data_fields_info) 
		if (std::count(fields.begin(), fields.end(), field_data.first)) 
			(*track_info).*metadata[field_data.first] = field_data.second;
}


void FLAC_read_track_info(const char *path_to_file, FLAC_track_info *track_info) {
	
	FLAC__StreamMetadata* tags;

	if (!path_to_file || !tags) { } //<------- 	exception!!!!!!!!!

	if (FLAC__metadata_get_tags(path_to_file, &tags)) {

		std::vector<std::string> data_fields = {};
		
		for (int tag_ind = 0; tag_ind < tags->data.vorbis_comment.num_comments; tag_ind++) 
			data_fields.push_back((char*)tags->data.vorbis_comment.comments[tag_ind].entry);


		std::map <std::string, std::string> data_fields_info;
		FLAC_extract_fields_info(&data_fields, &data_fields_info); //<-----!!!

		FLAC_fill_track_info(track_info, &data_fields_info);
	}
	else {} //<------- 	exception!!!!!!!!!
}


void FLAC_print_track_info(FLAC_track_info *track_info) {

	std::cout << "\n________________FLAC_FILE_INFO________________\n" << std::endl;
	for (auto& field_data : fields) 
		std::cout << field_data << " -- " << (*track_info).*metadata[field_data] << std::endl;
	
	std::cout << "_______________________________________________\n" << std::endl;

}

