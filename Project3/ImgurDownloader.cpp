#include <iostream>
#include <regex>
#include <vector>
#include <limits>
#include </Json/json.hpp>
#include <curl\curl.h>
#include "Download.h"
#include "Functions.h"

using std::cout; using std::endl; using std::vector;

void ImgurDownloader::getAllImages(string& raw_url) {
	getQueryString(raw_url);
	if (validate(raw_url)) {
		string url("https://api.imgur.com/3/album/" + this->query_string);
		string json_buffer;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_buffer);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

		curl_slist* header_list = nullptr;
		header_list = curl_slist_append(header_list, options.imgur_auth.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		auto response = curl_easy_perform(curl);
		curl_easy_reset(curl);

		if (response == CURLE_OK && !json_buffer.empty()) {
			getUrlsFromJson(json_buffer,urls);
			if (!urls.empty()) {
				std::for_each(urls.begin(), urls.end(), removeNonSupported);
				std::for_each(urls.begin(), urls.end(), [this](string& st) {
					changeImgToMp4(st);
				});

			}
			else {
				cout << "There was a problem parsing the data " << endl;
			}
		}
		else {
			cout << "An error has occured getting data from the url." << endl;
			cout << error << endl;
		}
	}
	else {
		cout << raw_url << " is not a valid url." << endl;
	}
	if (options.max_files <= urls.size()) {
		urls.resize(options.max_files);
	}

}


void ImgurDownloader::getUrlsFromJson(const string& buffer,vector<string>& output) {
	try {
		jsonp = json::parse(buffer.c_str());
		if (jsonp["success"]) {
			for (auto& image : jsonp["data"]["images"]) {
				if (min_size <= image.value("width",0) && min_size <= image.value("height",0) ) {
					output.push_back(image["link"]);
				}
			}
		}
		else {
			throw std::runtime_error("Expected success in gathering data, returned false, error code : " + jsonp["status"]);
		}
	}
	catch (std::exception& err) {
		cout << err.what() << endl;
	}
}


bool ImgurDownloader::validate(const string& url) {
	bool good = false;
	string pattern("(https://)?(www.)?imgur.com/((a)|(gallery))/.+");
	std::regex reg(pattern);
	good = std::regex_search(url, reg);
	if (!good) {
		pattern = "(https://)?(www.)?imgur.com/t/.+/.+";
		reg = pattern;
		good = std::regex_search(url, reg);
	}
	return good;
}


void ImgurDownloader::getQueryString(const string& url) {
	auto back_slash = std::find(url.rbegin(), url.rend(), '/').base();
	this->query_string = string(back_slash, url.cend());
}


void ImgurDownloader::websiteOptions(Options& opt) {
	cout << "What is the maximum amount of files wanted?" << endl;
	opt.max_files = check<int>("Only enter number", "Number must be bigger than 0", [](const int& i) {
		return i > 0;
	});

	cout << "What is the minimum dimension wanted? (0 for none)" << endl;
	this->min_size = check<int>("Only enter number", "Number must be bigger than 0", [](const int& i) {
		return i >= 0;
	});
}
