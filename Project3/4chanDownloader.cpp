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


void ChanDownloader::getUrlsFromJson(const string& json_data) {
	jsonp = json::parse(json_data.c_str());
	for (auto post_begin = jsonp["posts"].begin(); post_begin != jsonp["posts"].end(); post_begin++) {
		if (post_begin->find("w") != post_begin->end() && post_begin->find("h") != post_begin->end()
			&& post_begin->find("tim") != post_begin->end() && post_begin->find("ext") != post_begin->end()) { //making sure the post has media and not just a comment

			if (min_size < post_begin->value("w", 0) && min_size < post_begin->value("h", 0)) {
				urls.push_back("http://i.4cdn.org/" + chan_sub + "/" +
					std::to_string(post_begin->value("tim", unsigned long long int(0))) + post_begin->value("ext", ""));
			}

		}
	}
	
}

vector<string> ChanDownloader::getThreads(const string& page, const string& main_url) {
	vector<string> new_urls;
	try {
		jsonp = json::parse(page.c_str());
		auto threads = jsonp["threads"];
		for (auto board_threads = threads.begin(); board_threads != threads.end(); board_threads++) {
				string threadNumber = std::to_string((*board_threads)["posts"][0].value("no", 0));
				new_urls.push_back(main_url + "thread/" + threadNumber);
		}
	}
	catch (std::exception& err) {
		cout << "There was a problem getting the threads from your url." << endl;
		cout << err.what() << endl;
	};
	
	return new_urls;
}

void ChanDownloader::getChanSub(const string& url) {
	string pattern("(?:/)(.{1,5})(?:/)");
	std::smatch sm;
	std::regex reg(pattern);
	std::regex_search(url, sm, reg);
	this->chan_sub = sm[1].str();
}

void ChanDownloader::appendJsonString(string& url) const {
	string pattern("boards");
	string format("api");
	url = std::regex_replace(url, std::regex(pattern), format);
	char end_char = *(url.end() - 1);
	if (isdigit(end_char) && isdigit(*(url.end() - 2)) && end_char != '/') { //its a thread
		url.append(".json");
	}
	else { //its a board
		url.append(end_char == '/' ? "1.json" : ".json");
	}
}

bool ChanDownloader::validate(const string& url) {
	bool good = false;
	string pattern("(https?://)?boards\\.4chan\\.org/[a-zA-Z1-9]{1,4}/((\\d+)?|(thread/\\d+))(?!/)(?!#)");
	std::regex chan_regex(pattern);
	good = std::regex_match(url, std::smatch(),chan_regex);
	return good;
}

vector<string> ChanDownloader::getAllImages(string& url) {
	string base_url = url; //used to create multiple requests on the main board thats requested
	string::iterator iter = base_url.end() - 1, delete_iter = base_url.end() - 1;
	while (isdigit(*iter)) {
		delete_iter = iter--;
	}
	if (delete_iter != iter)
		base_url.erase(delete_iter, base_url.end()); //erase the number at the end if there is one.

	vector<string> pure_img;
	if (validate(url)) {
		appendJsonString(url);

		string pure_chan_json;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pure_chan_json);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
		curl_slist* header_list = nullptr;
		header_list = curl_slist_append(header_list, "User-Agent: 0"); //4chan will otherwise not allow access.
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

		auto response = curl_easy_perform(curl);
		curl_easy_reset(curl);

		if (response == CURLE_OK && !pure_chan_json.empty()) {
			getChanSub(url);
			if (std::regex_search(url, std::smatch(), std::regex("thread"))) { // if its a single thread
				try {
					Sleep(1950); // respecting 4chan's rules
					getUrlsFromJson(url);
				}
				catch (std::exception& err) {
					cout << "There was a problem getting the urls from your url." << endl;
					cout << err.what() << endl;
				}
			}
			else { //if its an entire board
				vector<string> newThreads = getThreads(pure_chan_json, base_url);
				if (!newThreads.empty()) { //newThread might have thrown an exception
					string thread_json;
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &thread_json);
					curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
					curl_slist* header_list = nullptr;
					header_list = curl_slist_append(header_list, "User-Agent: 0");
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

					for (string& threadUrl : newThreads) {
						Sleep(2000); // respecting 4chan API's rules.
						appendJsonString(threadUrl);
						curl_easy_setopt(curl, CURLOPT_URL, threadUrl.c_str());
						response = curl_easy_perform(curl);

						if (response == CURLE_OK && !thread_json.empty()) {
							try {
								getUrlsFromJson(thread_json);
							}
							catch (std::exception& err) {
								cout << "There was a problem getting the urls from your url after getting the threads." << endl;
								cout << err.what() << endl;
							}
						}
						else {
							cout << "Problem encountered proccessing thread : " << threadUrl << endl;
							cout << error << endl;
						}
						thread_json.clear();
					}
					curl_easy_reset(curl);
				}
			}// !else, board
			urls.erase(std::remove_if(urls.begin(), urls.end(), removeNonSupported), urls.end());
			pure_img.insert(pure_img.end(), std::make_move_iterator(urls.begin()), std::make_move_iterator(urls.end()));
			//move the elements.
			urls.clear();
		} // !if
		else {
			cout << error << endl;
			cout << "Encountered a problem with reading your url." << endl;
		}
	}
	else {
		cout << url << " is not a valid url." << endl;
	}
	if (pure_img.size() > options.max_files)
		pure_img.resize(options.max_files);
	return pure_img;
}



void ChanDownloader::websiteOptions(Options& opt) {
	cout << "What is the maximum amount of files wanted?" << endl;
	int d = check<int>("Invalid input, input only a number", "Your number is too large! Try again.", [](const int& i) { return i <= 10000 && i >0; });
	opt.max_files = d;

	cout << "What is the minimum size w/h of files? (0 for no limit)\nNote : this number represents the mininum for both width and height." << endl;
	int n = check<int>("Invalid input, input only a number", "Your number cannot contain a negative.", [](const int& i) {return i >= 0 ; });
	this->min_size = n;
}


#endif