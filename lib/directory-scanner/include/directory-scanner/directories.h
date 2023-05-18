#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include <iostream>
#include <vector>

// TODO: two identical files, collapse

namespace directories {

	enum types_ {FOLDER = 0, ENTRY = 1};

	class directory_content_obj {
	public:
		directory_content_obj() {};
		directory_content_obj(std::string path, std::string name, types_ type) : path_(path), name_(name), type_(type) {}

		std::string path_;
		std::string name_;
		types_ type_;
	};

	using directory_content_t = std::vector<directory_content_obj*>;

	class directory {
	public:
		directory() {};
		directory(std::string path) : path_(path) {};

		std::vector<directory_content_obj *> &get_content() {return content_;};

		void print_content() {
			for (const auto& obj: content_) 
				std::cout << obj->name_ << " : " << obj->type_ << std::endl;
		}

		std::string path_;
		directory_content_t content_;
	};
}

#endif
