#ifndef TUMBLR
#define TUMBLR
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include </Json/json.hpp>
#include <curl\curl.h>
#include "Download.h"
#include "Functions.h"


using std::string; using std::vector; using std::cout; using std::endl; using std::flush; 

vector<string> TumblrDownloader::getAllImages(string& raw_url) {
	vector<string> pure_imgs;
	getPureUrl(raw_url);
	if (validate(raw_url)) {
		/*
		Limit - The amount of posts in the request
		Offset - How many posts to move to
		*/
		for (int limit = 0, offset = 0; limit <= options.max_files; 
			offset +=20, limit+= options.max_files - offset > 20 ? 20 : options.max_files - offset ) {
			string url = "https://api.tumblr.com/v2/blog/" + pure_url + "/posts/photo?limit=" +
				std::to_string(limit) + "&offset=" + std::to_string(offset) + "&api_key=" + tumblr_auth;
			string json_buffer;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_buffer);
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
			auto response = curl_easy_perform(curl);
			curl_easy_reset(curl);
			if (response == CURLE_OK && !json_buffer.empty()) {
				getUrlsFromJson(json_buffer);
				std::remove_if(urls.begin(), urls.end(), removeNonSupported);
				std::for_each(urls.begin(), urls.end(), [&pure_imgs](const auto& item) {
					pure_imgs.push_back(std::move(item));
				});
				urls.clear();
			}
			else {
				cout << "There was a problem getting the images" << endl;
				cout << error << endl;
				break;
			}

		}
	}
	else {
		cout << raw_url << " is not a valid url" << endl;
	}
	return pure_imgs;
}



bool TumblrDownloader::validate(const string& url) {
	bool good = false;
	try {
		string pattern(".+\\.tumblr.com/?(.+)");
		std::regex reg(pattern);
		std::smatch s;
		bool base = std::regex_search(url,s, std::regex(pattern));
		bool extension = !s[1].matched;
		good = base && extension;
	}
	catch (std::regex_error& err) {
		cout << err.what() << endl;
	}

	if (!good) { // it might just be a custom hostname

		//send a request to the server, it will reply if the hostname is apart of the tumblr server.
		string json_buffer; 
		string test_url = "https://api.tumblr.com/v2/blog/" + pure_url + "/posts/photo?limit=1&api_key=" + tumblr_auth;
		curl_easy_setopt(curl, CURLOPT_URL, test_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_buffer);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
		auto response = curl_easy_perform(curl);
		curl_easy_reset(curl);
		if (response == CURLE_OK) {
			json json_data = json::parse(json_buffer.c_str());
			good = json_data["meta"]["status"] == 200;
		}
		else {
			cout << error << endl;
		}
	}

	return good;
}

void TumblrDownloader::getPureUrl(const string& url) {
	// remove the http and other prefixes
	string pattern("(https?://)?(www.)?(.+)");
	string replace("$3");
	std::regex reg(pattern);
	std::smatch s;
	this->pure_url = std::regex_replace(url, reg, replace);
	char backslash = *(pure_url.end() - 1);
	if (backslash == '/')
		pure_url.pop_back();
}

void TumblrDownloader::getUrlsFromJson(const string& url) {
	try {
		jsonp = json::parse(url.c_str());
		auto all_posts = jsonp["meta"]["posts"];
		for (auto& single : all_posts) {
			auto photo_obj = single["photos"][0]["original_size"];
			if (min_size <= photo_obj.value("width", 0) && min_size <= photo_obj.value("height", 0)) {
				urls.push_back(photo_obj["url"]);
			}
		}
	}
	catch (std::exception& err) {
		cout << err.what() << endl;
	}
	
}


void TumblrDownloader::websiteOptions(Options& options) {

}
#endif
