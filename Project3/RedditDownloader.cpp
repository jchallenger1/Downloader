#ifndef REDDITDOWNLOADER
#define REDDITDOWNLOADER
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <regex>
#include <filesystem>
#include "Download.h"
#include "Functions.h"
using std::cin; using std::cout; using std::endl;


void RedditDownloader::getUrlsFromJson(const string& data, vector<string>& output) {
	jsonp = json::parse(data.c_str());
	auto n = jsonp["data"]["children"];
	auto begin = n.begin();
	auto end = n.end();
	while (begin != end) {
		output.push_back((*begin)["data"]["url"]);
		begin++;
	}
}

void RedditDownloader::nextPage(string& query_string,string& url) const {

	if (std::regex_search(url, std::smatch(), std::regex("after"))) {
		string pattern("after=.+(?!&)");
		string format("after=" + query_string);
		std::regex reg(pattern);
		url = std::regex_replace(url, reg, format);
	}
	else {
		auto question = std::find(url.begin(), url.end(), '?');
		if (question != url.end())
			url.append("&after=" + query_string);
		else
			url.append("?after=" + query_string);
	}
}


void RedditDownloader::appendJsonString(string& url) const {
	string::iterator question = std::find(url.begin(), url.end(), '?');
	if (question != url.end()) {
		string query(question, url.end());
		url.erase(question, url.end());
		url.append(".json");
		url.append(query);
	}
	else {
		url = url[url.size() - 1] == '/' ? url : url += "/";
		url.append(".json");
	}
	
}

void RedditDownloader::getAllImages(string& url) {
	int page_count = options.page_count;
	if (validate(url)) {
		
		appendJsonString(url);
		while (page_count > 0 && urls.size() < options.max_files) {
			string all_reddit_json;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &all_reddit_json);
			auto response = curl_easy_perform(curl);
			curl_easy_reset(curl);

			if (response == CURLE_OK && !all_reddit_json.empty()) {
				vector<string> image_urls;

				try {
					getUrlsFromJson(all_reddit_json,image_urls); // parsing the json and getting the urls.
				}
				catch (const std::exception&) {
					cout << "There was a problem getting data with your url." << endl;
					break;
				}
				
				image_urls.erase(std::remove_if(image_urls.begin(), image_urls.end(), removeNonSupported), image_urls.end()); //remove urls that are not in correct format.
				auto mapped_urls = mapUrls(image_urls); //making all urls to make sure they are in the format of only an image
				std::for_each(mapped_urls.begin(), mapped_urls.end(), [&](std::pair<string,string>& pa) { // changes a gif  or gifv extension to mp4 for better optimization.
						if (pa.second == "GOOD")
							changeImgToMp4(pa.first);
				});
				vector<string> new_urls = getPureImgUrl(mapped_urls); // on sites that do not have a pure image, we obtain them here.
				auto fx = [](const std::pair<string, string>& pair_url) {return pair_url.second == "DELETE" ? true : false; };
				mapped_urls.erase(std::remove_if(mapped_urls.begin(), mapped_urls.end(), fx), mapped_urls.end()); //remove the non pure images
				std::copy(new_urls.begin(), new_urls.end(), std::back_insert_iterator<vector<string>> (image_urls)); //add them to the main vector with all pure images
				std::for_each(mapped_urls.begin(), mapped_urls.end(), [this](std::pair<string,string>& item) { // add them to the this urls.
					this->urls.push_back(std::move(item.first)); // preventing copy
				});

			}
			else {
				std::cout << (strlen(error) == 0 ? "An error occured with your url!" : error) << std::endl;
				cout << url << endl;
				break;
			}
			if (jsonp["data"]["after"].is_null()) { // no more pages to search
				break;
			}
			else {
				string next_page_query = jsonp["data"]["after"];
				nextPage(next_page_query,url);
			}
			page_count--;
		} // !while
	} // !validate if
	else {
		cout << url << "\n is not a valid url." << endl;
	}
	if (urls.size() > options.max_files) {
		urls.resize(options.max_files);
	}

}

bool RedditDownloader::validate(const string& url) {
	string pattern("https?://www.reddit.com/r/\\w+(/(?!comments))?");
	std::regex reg(pattern);
	bool a = std::regex_search(url, reg);
	return a;
}

void RedditDownloader::websiteOptions(Options& opt) {

	cout << "How many pages do you want? Max of 50." << endl;
	int pages = check<int>("Invalid input enter amt of pages 1-50.", "Only numbers between 1-50.", [](const int& i) {return i <= 50&& i >0; });
	opt.page_count = pages;

	cout << "What is the maximum amount of files wanted?" << endl;
	int d = check<int>("Invalid input, input only a numbers", "Your number is too large or to small! Try again.", [](const int& i) { return i <= 10000 && i >0; });
	opt.max_files = d;

	cout << "Do you want to gather all images when faced with an entire gallery? (y/n)" << endl;
	char all = check<char>("Invalid input, only input (y)es or (n)o","Only input (y)es or (n)o",yesOrNo );
	opt.all_gallery = all == 'y' ? true : false;

}

#endif // !REDDITDOWNLOADER