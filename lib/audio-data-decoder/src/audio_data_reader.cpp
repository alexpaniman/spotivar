#include "FLAC/metadata.h"
#include "json/json.h"

#include "audio-data-decoder/audio_data.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <cctype>

std::string simple_bites_reader(FILE *mp3_file, int num_of_bytes, int readloc = SEEK_CUR-1) {

	std::string buffer(num_of_bytes, '\0');

	fseek(mp3_file, readloc, SEEK_SET); //?
	fread(&buffer[0], 1, num_of_bytes, mp3_file);

	return buffer;
}


bool check_symbol(char i) {
	return i && (int(i) > 31);
}


std::vector<std::string> get_words(std::string text_with_garbage) {

	std::vector<std::string> words = {""};

	for (char i: text_with_garbage) {
		if (check_symbol(i)) {
			words.back()+=i;
		}
		else {
			if (words.back().length() != 0) words.push_back("");
		}
	}
	return words;
}


static std::vector<std::string> id3v1_tags = {"Title", "Artist", "Album", "Year", "Comment", "Track" , "Genre"};

static std::vector<std::string> id2v2_2_tags =                    
	{"BUF", "CNT", "COM", "CRA", "CRM"  , "ETC", "EQU", "GEO",    
	"IPL", "LNK", "MCI", "MLL", "PIC", "POP", "REV", "RVA", "SLT",
	"STC", "TAL", "TBP", "TCM", "TCO", "TCR", "TDA", "TDY", "TEN",
	"TFT", "TIM", "TKE", "TLA", "TLE", "TMT", "TOA", "TOF", "TOL",
	"TOR", "TOT", "TP1", "TP2", "TP3", "TP4", "TPA", "TPB", "TRC",
	"TRD", "TRK", "TSI", "TSS", "TT1", "TT2", "TT3", "TXT", "TXX",
	"TYE", "UFI", "ULT", "WAF", "WAR", "WAS", "WCM", "WCP", "WPB",
	"WXX"};


static std::vector<std::string> id3v2_3_tags =                     
	{"COMM", "COMR", "ENCR", "EQUA", "ETCO", "GEOB", "GRID", "IPLS",
	"LINK", "MCDI", "MLLT", "OWNE", "PRIV", "PCNT", "POPM", "POSS",
	"RBUF", "RVAD", "RVRB", "SYLT", "SYTC", "TALB", "TBPM", "TCOM",
	"TCON", "TCOP", "TDAT", "TDLY", "TENC", "TEXT", "TFLT", "TIME",
	"TIT1", "TIT2", "TIT3", "TKEY", "TLAN", "TLEN", "TMED", "TOAL",
	"TOFN", "TOLY", "TOPE", "TORY", "TOWN", "TPE1", "TPE2", "TPE3",
	"TPE4", "TPOS", "TPUB", "TRCK", "TRDA", "TRSN", "TRSO", "TSIZ",
	"TSRC", "TSSE", "TYER", "TXXX", "UFID", "USER", "USLT", "WCOM",
	"WCOP", "WOAF", "WOAR", "WOAS", "WORS", "WPAY", "WPUB", "WXXX"};


static std::vector<std::string> id3v2_4_tags =                    
	{"AENC", "APIC", "ASPI", "COMM", "COMR", "ENCR", "EQU2", "ETCO",
	"GEOB", "GRID", "LINK", "MCDI", "MLLT", "OWNE", "PRIV", "PCNT",
	"POPM", "POSS", "RBUF", "RVA2", "RVRB", "SEEK", "SIGN", "SYLT",
	"SYTC", "TALB", "TBPM", "TCOM", "TCON", "TCOP", "TDEN", "TDLY",
	"TDOR", "TDRC", "TDRL", "TDTG", "TENC", "TEXT", "TFLT", "TIPL",
	"TIT1", "TIT2", "TIT3", "TKEY", "TLAN", "TLEN", "TMCL", "TMED",
	"TMOO", "TOAL", "TOFN", "TOLY", "TOPE", "TOWN", "TPE1", "TPE2",
	"TPE3", "TPE4", "TPOS", "TPRO", "TPUB", "TRCK", "TRSN", "TRSO",
	"TSOA", "TSOP", "TSOT", "TSRC", "TSSE", "TSST", "TXXX", "WCOM",
	"WCOP", "WOAF", "WOAR", "WOAS", "WORS", "WPAY", "WPUB", "WXXX"};


static std::map<std::string, std::vector<std::string>> id3_main_tags = {
	{"TITLE", {"TIT2", "TT2", "Title"}},
	{"ARTIST", {"TPE1", "TP1", "Artist"}},
	{"ALBUM", {"TALB", "TAL", "Album"}},
	{"COMPOSER", {"TCOM", "TCM"}},
	{"YEAR", {"Year", "TYE", "TYER", "TDRC"}},
	{"DATE", {"TDA", "TDAT", "TDOR", "TDRL"}},
	{"PERFORMER", {"TPE2", "TPE3", "TPE4", "TP2", "TP3", "TP4"}},
	{"GENRE", {"TCON", "TCO", "Genre"}},
	{"LENGTH", {"TLE", "TLEN"}},
	{"COMMENT", {"COM", "COMM", "Comment"}},
	{"PIC", {"PIC"}},
	{"DESCRIPTION", {"TIT3", "TT3"}}
};


static std::map<std::string, std::string audio::track_info::*> metadata = {
	{"TITLE", &audio::track_info::title},
	{"ARTIST", &audio::track_info::artist},
	{"ALBUM", &audio::track_info::album},
	{"COMPOSER", &audio::track_info::composer},
	{"YEAR", &audio::track_info::year},
	{"DATE", &audio::track_info::date},
	{"PERFORMER", &audio::track_info::performer},
	{"GENRE", &audio::track_info::genre},
	// {"LENGTHaudio::", &mp3_info::length},
	{"COMMENT", &audio::track_info::comment},
	// {"PICaudio::", &mp3_info::pic},
	{"DESCRIPTION", &audio::track_info::description}
};


template<typename T> 
bool element_is_contained(T elem, std::vector<T> vec) {

	if (vec.size()) 
		return std::find(vec.begin(), vec.end(), elem) != vec.end();
	return false;
}


std::map<std::string, std::string> extract_tag_values(std::vector<std::string>& words) {

	std::string tag = "";
	std::map<std::string, std::string> tag_values{{"", ""}};

	for (std::string i: words) {
		if (element_is_contained(i, id3v2_4_tags) || element_is_contained(i, id3v2_3_tags) || \
			element_is_contained(i, id2v2_2_tags) || element_is_contained(i, id3v1_tags)) {

			tag_values[i] = "";
			tag = i;
		}
		else
			tag_values[tag]+=i;
	}

	return tag_values;
}


void fill_mp3_info(audio::mp3_data* track, std::map<std::string, std::string>& tags) {

	for (auto& tag: tags) {
		for (auto& main_tag: id3_main_tags) {
			if (std::find(main_tag.second.begin(), main_tag.second.end(), tag.first) != main_tag.second.end()) {
				track->info_.*(metadata[main_tag.first]) = tag.second;
			}
		}
	}
}


bool check_mp3_format(FILE *mp3_file) {

	if (simple_bites_reader(mp3_file, 3) == "ID3")
		return true;

	return false;
}


audio::mp3_data::mp3_data(const char *file_name) {
	FILE* file = fopen(file_name, "r");

	if (!file || !check_mp3_format(file)) throw std::invalid_argument("STRANGE PATH..... >:("); //<----- AAAAAAAAAAA!!!!!

	std::string tags_without_header = simple_bites_reader(file, 600, 10);
	std::vector<std::string> words = get_words(tags_without_header);

	std::map<std::string, std::string> tag_values = extract_tag_values(words);
	fill_mp3_info(this, tag_values);

	fclose(file);
}


std::string make_string_large(std::string str) {

	for (int i = 0; i < str.length(); i++)
		str[i] = toupper(str[i]);

	return str;
}


bool FLAC_extract_fields_info(std::vector<std::string> *data_fields, std::map <std::string, std::string> *data_fields_info) {

	const char separator = '=';
	int separator_position;

	for (std::string data_field : *data_fields ) {

		separator_position = data_field.find(separator);
		(*data_fields_info)[make_string_large(data_field.substr(0, separator_position))] = data_field.substr(separator_position+1, data_field.length()-1);
		// std::cout << (*data_fields_info)[data_field.substr(0, separator_position)] << std::endl;
	}
	
	return true; //<------- !!!!!
}

static const std::vector<std::string> fields = {"TITLE", "ARTIST", "ALBUM", "COMMENT", "COMPOSER", "DATE", "GENRE", "PERFORMER"};

void FLAC_fill_track_info(audio::flac_data *track, std::map <std::string, std::string> *data_fields_info) {

	for (auto& field_data : *data_fields_info) {
		if (element_is_contained(make_string_large(field_data.first), fields)) 
			track->info_.*(metadata[field_data.first]) = field_data.second;
	}
}


audio::flac_data::flac_data(const char *file_name) {
	
	FLAC__StreamMetadata* tags;

	if (!file_name || !tags) { } //<------- 	exception!!!!!!!!!

	if (FLAC__metadata_get_tags(file_name, &tags)) {

		std::vector<std::string> data_fields = {};
		
		for (int tag_ind = 0; tag_ind < tags->data.vorbis_comment.num_comments; tag_ind++) {
			data_fields.push_back((char*)tags->data.vorbis_comment.comments[tag_ind].entry);
			// std::cout << tag_ind << std::endl;
			// std::cout << tags->data.vorbis_comment.comments[tag_ind].entry;
		} 

		std::map <std::string, std::string> data_fields_info;
		FLAC_extract_fields_info(&data_fields, &data_fields_info); //<-----!!!

		FLAC_fill_track_info(this, &data_fields_info);
	}
	else {} //<------- 	exception!!!!!!!!!

	file_path_ = file_name;
}


void audio::audio_data::print_file_info() {

	std::cout << "\n__________________FILE_INFO________________\n" << std::endl;
	for (auto& field : metadata) 
		std::cout << field.first << " -- " << info_.*metadata[field.first] << std::endl;
	
	std::cout << "_____________________________________________\n" << std::endl;

}

void audio::audio_data::print_track_info() {

	std::cout << "\n_________________AUDIO_INFO________________\n" << std::endl;
	for (auto& field : metadata) 
		std::cout << field.first << "\t\t" << info_.*metadata[field.first] << std::endl;
	
	std::cout << "\nHASH\t" << hash_ << std::endl;
	std::cout << "\nPATH\t" << file_path_ << std::endl;
	std::cout << "\nTAGS\t" << std::endl;
	for (int i = 0; i < tags_.size(); i++) 
		std::cout << "\t\t" << tags_[i] << std::endl;
	// std::cout << tags_.size();
	std::cout << "_____________________________________________\n" << std::endl;

}


audio::audio_data::audio_data(Json::Value json_obj) {
	//validate!!!
	for (auto& field : metadata) 
		info_.*metadata[field.first] = json_obj["file_info"][field.first].asString();

	file_path_ = json_obj["file_path"].asString();
	hash_ = json_obj["hash"].asUInt64();

	int tag_count = json_obj["tags"].size();

	const Json::Value tags = json_obj["tags"];
	for (int i = 0; i < tag_count; i++)
		tags_.push_back(tags[i].asString());

}

Json::Value audio::audio_data::get_json_info() {

	Json::Value track;

	for (auto& field : metadata) {
		track["file_info"][field.first] = info_.*metadata[field.first];
	}
	track["file_path"] = file_path_;
	track["hash"] = hash_;

	Json::Value tags(Json::arrayValue);
	for (auto& tag: tags_) 
		tags.append(tag);
	track["tags"] = tags;

	return track;
}

void audio::audio_data::write(const char *path) {

	std::ofstream file_id;
	file_id.open(path);

	Json::Value track = get_json_info();

	Json::StyledWriter styledWriter;
	file_id << styledWriter.write(track);

	file_id.close();
}
