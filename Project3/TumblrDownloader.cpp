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

/*
Note - Tubmlr Api does not provide any information for pages
       They only provide information based off of singular posts.
*/

using std::string; using std::vector; using std::cout; using std::endl; using std::flush; 

void TumblrDownloader::getAllImages(string& raw_url) {
	
	getPureUrl(raw_url);
	if (validate(raw_url)) {
		/*
		Limit - The amount of posts in the request
		Offset - How many posts to set/move to
		*/
		for (int limit = 0, offset = 0; limit <= options.page_count; 
			offset +=20, limit+= options.page_count - offset > 20 ? 20 : options.page_count - offset ) {// until we've requested for all images
			vector<string> pure_imgs;
			if (urls.size() >= options.max_files) {
				break;
			}

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
				getUrlsFromJson(json_buffer, pure_imgs);
				if (pure_imgs.empty()) {
					break;
				}
				std::remove_if(pure_imgs.begin(), pure_imgs.end(), removeNonSupported);
				std::for_each(pure_imgs.begin(), pure_imgs.end(), [this](const auto& item) {
					urls.push_back(std::move(item));
				});

			}
			else {
				cout << "There was a problem getting the images" << endl;
				cout << error << endl;
				break;
			}

		} // !for 
	}
	else {
		cout << raw_url << " is not a valid url" << endl;
	}
	if (urls.size() >= options.max_files) {
		urls.resize(options.max_files);
	}

}



bool TumblrDownloader::validate(const string& url) {
	bool good = false;
	string pattern(".+\\.tumblr.com/?(.+)");
	std::regex reg(pattern);
	std::smatch s;
	bool base = std::regex_search(url,s, std::regex(pattern));
	bool extension = !s[1].matched;
	good = base && extension;

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

void TumblrDownloader::getUrlsFromJson(const string& url,vector<string>& output) {
	try {
		jsonp = json::parse(url.c_str());
		auto all_posts = jsonp["response"]["posts"];
		for (auto& single_post : all_posts) {
			if (!options.tag.empty()) {
				if (!hasCorrectTag(single_post["tags"])) {
					continue;
				}
			}
			if (options.all_gallery) {
				for (auto& photos : single_post["photos"]) {
					std::pair<int, int> dimensions({ photos["original_size"]["width"],photos["original_size"]["height"] });
					if (min_size <= dimensions.first && min_size <= dimensions.second) {
						string img_url = photos["original_size"]["url"];
						output.push_back(img_url);
					}
				}
			}
			else {
				std::pair<int, int> dimensions({ single_post["photos"][0]["original_size"]["width"],single_post["photos"][0]["original_size"]["height"] });
				if (min_size <= dimensions.first && min_size <= dimensions.second) {
					output.push_back(single_post["photos"][0]["original_size"]["url"]);
				}
			}
			
		} // !for
	}
	catch (std::exception& err) {
		cout << err.what() << endl;
	}
	
}

template<typename T>
bool TumblrDownloader::hasCorrectTag(T& tags) const{
	bool hasTag = false;
	for (auto& t : tags) {
		if (options.tag == t)
			hasTag = true;
	}
	return hasTag;
}

void TumblrDownloader::websiteOptions(Options& options) {
	cout << "How many posts should be searched? (NOT PAGES!)" << endl;
	options.page_count = check<int>("Only enter numbers", "Number has to be atleast 0, less than 10000", [](const int& i) {
		return i >= 0 && i <= 10000;
	});

	cout << "What is the maximum amount of media downloaded?" << endl;
	options.max_files = check<int>("Only enter numbers", "Number has to be greater than 0", [](const int& i) {
		return i >= 0;
	});

	cout << "What is the minimum dimension of the media required?(Enter 0 for none)" << endl;
	this->min_size = check<int>("Only enter numbers", "Number has to be atleast 0", [](const int& i) {
		return i >= 0;
	});

	cout << "Do you want all images in a single post (1), or just one (2)?" << endl;
	this->options.all_gallery = check<int>("Only enter numbers", "Number has to be 1 or 2", [](const int& i) {return i == 1 || i == 2; });

	cout << "Do you want to search for a specific tag(y/n)?" << endl;
	char search = check<char>("Only enter a character", "Enter (y) for yes, (n) for no", yesOrNo);
	if (search == 'y') {
		while (true) {
			cout << "Enter your tag" << endl;
			string input = checkGetLine<string>("", "");
			std::cin.ignore();
			cout << "Your entered tag is '" << input << "'\nIs this correct(y/n)?" << endl;
			char good = check<char>("Only enter characters", "Enter (y) for yes, (n) for no", yesOrNo);
			if (good == 'y') {
				options.tag = input;
				break;
			}
		}
	}
}
#endif
