#ifndef CHAN
#define CHAN

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cctype>
#include <iterator>
#include </Json/json.hpp>
#include <curl/curl.h>
#include "Download.h"
#include "Functions.h"

using std::string; using std::cin; using std::cout; using std::endl; using json = nlohmann::json;

vector<string> ChanDownloader::getAllImages(string& url) {
	vector<string> pure_img;
	if (validate(url)) {
		appendJsonString(url);
		string pure_chan_json;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pure_chan_json);
		auto response = curl_easy_perform(curl);
		if (response == CURLE_OK && !pure_chan_json.empty()) {
			getChanSub(url);
			vector<string> thread_urls = processThread(pure_chan_json);
			std::move_iterator<vector<string>::iterator> move_begin = std::make_move_iterator(thread_urls.begin());
			std::move_iterator<vector<string>::iterator> move_end = std::make_move_iterator(thread_urls.end());
			pure_img.reserve(pure_img.size() + thread_urls.size());
			pure_img.insert(pure_img.end(), move_begin, move_end);
		}
		else {
			cout << "Encountered a problem with reading your url." << endl;
		}
	}
	else {
		cout << url << " is not a valid url." << endl;
	}
	return pure_img;
}

vector<string> ChanDownloader::processThread(const string& json_data) {
	vector<string> url;
	jsonp = json::parse(json_data.c_str());
	auto posts = jsonp["threads"]["posts"];
	auto begin = posts.begin(), end = posts.end();
	while (begin != end) {
		if (begin->find("tim") != begin->end() && begin->find("ext") != begin->end())
			url.emplace_back("http://i.4cdn.org/" + chan_sub + "/" + begin->value("tim", "") + begin->value("ext", ""));
		begin++;
	}
	return url;
}

void ChanDownloader::getChanSub(const string& url) {
	string pattern("(?=4chan.org/).+?(?=/)");
	std::smatch sm;
	std::regex reg(pattern);
	std::regex_search(url, sm, reg);
	this->chan_sub = sm.str();
}

void ChanDownloader::appendJsonString(string& url) {
	string pattern("boards");
	string format("api");
	std::regex_replace(url, std::regex(pattern), format);
	char end_char = *(url.end() - 1);
	if (isdigit(end_char) && end_char != '/') { //its a thread
		url.append(".json");
	}
	else { //its a board
		url.append(end_char == '/' ? "1.json" : ".json");
	}
}

bool ChanDownloader::validate(const string& url) {
	bool good = false;
	try {
		string pattern("(https?://)?boards\\.4chan\\.org/[a-zA-Z1-9]{1,4}/((\\d+)?|(thread/\\d+))(?!/)(?!#)");
		std::regex chan_regex(pattern);
		good = std::regex_match(url, std::smatch(),chan_regex);
	}
	catch (std::regex_error err) {
		cout << err.what() << endl;
	}
	return good;
}

#endif

#ifndef CHANFUN
#define CHANFUN

bool chanOptions(Options&) {
	return false;
}

void runChanDownloader(string& current_dir) {

}

#endif