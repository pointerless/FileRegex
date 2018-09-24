#pragma once

#include <regex>
#include <fstream>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class FileRegexType {
		Exact,
		Repeated,
		Unordered
};

class FileRegex {
	public:
		explicit FileRegex(const std::string& filename){
			std::ifstream file(filename);
			std::string file_str((std::istreambuf_iterator<char>(file)),
			                     std::istreambuf_iterator<char>());
			json file_json = json::parse(file_str.c_str());

			if(file_json["pattern_type"].is_string()){ //Load pattern type
				if(file_json["pattern_type"] == std::string("Exact")){
					this->type = FileRegexType::Exact;
				}else if(file_json["pattern_type"] == std::string("Repeated")){
					this->type = FileRegexType::Repeated;
				}else if(file_json["pattern_type"] == std::string("Unordered")){
					this->type = FileRegexType::Unordered;
				}
			}

			if(file_json["patterns"].is_array()){       //Load patterns
				std::vector <std::string> patterns = file_json["patterns"];
				for(const std::string &str : patterns){
					this->linepatterns.push_back(std::regex(str));
				}
			}

			if(file_json["extensions"].is_array()){     //Load extensions
				std::vector <std::string> ext = file_json["extensions"];
				this->validextensions = ext;
			}

			if(file_json["line_size_range"].is_array()){
				std::vector <uint64_t> size_range = file_json["line_size_range"];
				if(size_range.size() == 1){ this->maxlength = size_range[0]; }
				else if(size_range.size() == 2){
					this->minlength = size_range[0];
					this->maxlength = size_range[1];
					if(this->minlength >= this->maxlength){
						throw std::logic_error("line_size_range in file " + filename + " invalid");
					}
				}
			}

			if(file_json["file_length_range"].is_array()){
				std::vector <uint64_t> length_range = file_json["file_length_range"];
				if(length_range.size() == 1){ this->maxlines = length_range[0]; }
				else if(length_range.size() == 2){
					this->minlines = length_range[0];
					this->maxlines = length_range[1];
					if(this->minlines >= this->maxlines){
						throw std::logic_error("file_length_range in file " + filename + " invalid");
					}
				}
			}

			if(file_json["always_match"].is_array()){
				std::vector <std::string> match = file_json["always_match"];
				for(const std::string &pattern : match){
					this->alwaysmatch.push_back(std::regex(pattern));
				}
			}

			if(file_json["never_match"].is_array()){
				std::vector <std::string> notmatch = file_json["never_match"];
				for(const std::string &pattern : notmatch){
					this->alwaysmatch.push_back(std::regex(pattern));
				}
			}

			if(file_json["include_blank"].is_boolean()){
				this->include_blank = file_json["include_blank"];
			}
		}

		bool matchRawFile(const std::string& filename) const{
			for(const std::regex &inc : this->alwaysmatch){        //Check if always match
				if(std::regex_match(filename, inc)){
					return true;
				}
			}
			for(const std::regex &exc : this->alwaysnotmatch){     //Check if always not match
				if(std::regex_match(filename, exc)){
					return false;
				}
			}

			bool matches = false;

			if(this->validextensions.size() > 0){
				const auto filnmsplt = strSplit(filename, '.');    //Match file extension
				for(const std::string &ext : this->validextensions){
					if(filnmsplt[filnmsplt.size() - 1] == ext){
						matches = true;
						break;
					}
				}
				if(!matches){ return false; }
			}else{
				matches = true;
			}

			std::ifstream file(filename);
			std::string line;
			std::size_t linecount = 0;
			while(getline(file, line)){
				if(this->maxlength != 0){
					if(line.size() < this->minlength || line.size() > this->maxlength){
						return false;
					}
				}
				if(!include_blank && line.size() == 0){ continue; }
				switch(type){
					case FileRegexType::Exact:{
						if(linecount == this->linepatterns.size()){
							matches = false;
						}else if(!std::regex_match(line, this->linepatterns[linecount])){
							matches = false;
						}
						break;
					}
					case FileRegexType::Repeated:{
						if(linecount == this->linepatterns.size()){
							linecount = 0;
						}
						if(!std::regex_match(line, this->linepatterns[linecount])){
							matches = false;
						}
						break;
					}
					case FileRegexType::Unordered:{
						for(std::size_t pos = 0; pos < this->linepatterns.size(); pos++){
							if(std::regex_match(line, this->linepatterns[pos])){
								break;
							}
						}
						matches = false;
						break;
					}
					default:{
						return false;
					}
				}
				if(!matches){ return false; }
				linecount++;
			}
			if(this->maxlines > 0){
				if(linecount < this->minlines || linecount > this->maxlines){
					return false;
				}
			}
			return true;
		}

		bool matchString(const std::string& str, const std::string& filename) const{
			for(const std::regex &inc : this->alwaysmatch){        //Check if always match
				if(std::regex_match(filename, inc)){
					return true;
				}
			}
			for(const std::regex &exc : this->alwaysnotmatch){     //Check if always not match
				if(std::regex_match(filename, exc)){
					return false;
				}
			}

			bool matches = false;

			if(this->validextensions.size() > 0){
				const auto filnmsplt = strSplit(filename, '.');    //Match file extension
				for(const std::string &ext : this->validextensions){
					if(filnmsplt[filnmsplt.size() - 1] == ext){
						matches = true;
						break;
					}
				}
				if(!matches){ return false; }
			}else{
				matches = true;
			}

			std::stringstream file;
			file << str;
			std::string line;
			std::size_t linecount = 0;
			while(getline(file, line)){
				if(this->maxlength != 0){
					if(line.size() < this->minlength || line.size() > this->maxlength){
						return false;
					}
				}
				if(!include_blank && line.size() == 0){ continue; }
				switch(type){
					case FileRegexType::Exact:{
						if(linecount == this->linepatterns.size()){
							matches = false;
						}else if(!std::regex_match(line, this->linepatterns[linecount])){
							matches = false;
						}
						break;
					}
					case FileRegexType::Repeated:{
						if(linecount == this->linepatterns.size()){
							linecount = 0;
						}
						if(!std::regex_match(line, this->linepatterns[linecount])){
							matches = false;
						}
						break;
					}
					case FileRegexType::Unordered:{
						for(std::size_t pos = 0; pos < this->linepatterns.size(); pos++){
							if(std::regex_match(line, this->linepatterns[pos])){
								break;
							}
						}
						matches = false;
						break;
					}
					default:{
						return false;
					}
				}
				if(!matches){ return false; }
				linecount++;
			}
			if(this->maxlines > 0){
				if(linecount < this->minlines || linecount > this->maxlines){
					return false;
				}
			}
			return true;
		}

	private:
		static std::vector <std::string> strSplit(const std::string &str, char delim = '\n'){
			std::vector <std::string> result;
			std::stringstream ss;
			ss << str;
			std::string item;
			while(std::getline(ss, item, delim)){
				result.push_back(item);
			}
			return result;
		}

		FileRegexType type;
		std::vector <std::string> validextensions;
		std::vector <std::regex> linepatterns;
		unsigned int minlines = 0;
		unsigned int maxlines = 0;
		unsigned int minlength = 0;
		unsigned int maxlength = 0;
		std::vector <std::regex> alwaysmatch;
		std::vector <std::regex> alwaysnotmatch;
		bool include_blank;
};
