#ifndef REDDITDOWNLOADER
#define REDDITDOWNLOADER
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <regex>
#include <filesystem>
#include "Download.h"
#include "Functions.h"
#define REDDIT_ERROR 0
using std::cin; using std::cout; using std::endl;


void RedditDownloader::getUrlsFromJson(string& data) {
	jsonp = json::parse(data.c_str());
	auto n = jsonp["data"]["children"];
	auto begin = n.begin();
	auto end = n.end();
	while (begin != end) {
		urls.push_back((*begin)["data"]["url"]);
		begin++;
	}
}

void RedditDownloader::nextPage(string& query_string,string& url) {

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


void RedditDownloader::appendJsonString(string& url) {
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

vector<string> RedditDownloader::getAllImages(string& url) {
	vector<string> image_urls;
	int page_count = options.page_count;
	if (validate(url)) {
		
		appendJsonString(url);
		while (page_count != 0 && image_urls.size() < options.max_files) {
			string all_reddit_json;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &all_reddit_json);
			auto response = curl_easy_perform(curl);
			curl_easy_reset(curl);
			if (response == CURLE_OK && !all_reddit_json.empty()) {
				try {
					getUrlsFromJson(all_reddit_json); // parsing the json and getting the urls.
				}
				catch (const std::exception&) {
					cout << "There was a problem getting data with your url." << endl;
					break;
				}
				
				urls.erase(std::remove_if(urls.begin(), urls.end(), removeNonSupported), urls.end()); //remove urls that are not in correct format.
				auto mapped_urls = mapUrls(urls); //making all urls to make sure they are in the format of only an image
				std::for_each(mapped_urls.begin(), mapped_urls.end(), [&](std::pair<string,string>& pa) { // changes a gif  or gifv extension to mp4 for better optimization.
						if (pa.second == "GOOD")
							changeImgToMp4(pa.first);
				});
				auto new_urls = getPureImgUrl(mapped_urls); // on sites that do not have a pure image, we obtain them here.
				std::function<bool(const std::pair<string, string>&)> fx = [](const std::pair<string, string>& pair_url) {return pair_url.second == "DELETE" ? true : false; };
				mapped_urls.erase(std::remove_if(mapped_urls.begin(), mapped_urls.end(), fx), mapped_urls.end()); //remove the non pure images
				std::copy(new_urls.begin(), new_urls.end(), std::back_insert_iterator<vector<string>> (image_urls)); //add them to the main vector with all pure images
				std::for_each(mapped_urls.begin(), mapped_urls.end(), [&image_urls](std::pair<string,string>& item) { // add them, same as above.
					image_urls.push_back(std::move(item.first)); // preventing copy
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
			urls.clear();
			page_count--;
		} // !while
	} // !validate if
	else {
		cout << url << "\n is not a valid url." << endl;
	}
	if (image_urls.size() > options.max_files) {
		return vector<string>(image_urls.begin(), image_urls.begin() + options.max_files);
	}
	else {
		return image_urls;
	}
}

bool RedditDownloader::validate(const string& url) {
	string pattern("https?://www.reddit.com/r/\\w+(/(?!comments))?");
	std::regex reg(pattern);
	bool a = std::regex_search(url, reg);
	return a;
}



#endif // !REDDITDOWNLOADER

#ifndef REDDITFUNCTIONS
#define REDITFUNCTION

void redditOptions(Options& opt) {

	cout << "How many pages do you want? Max of 50." << endl;
	int pages = check<int>("Invalid input enter amt of pages 1-50.", "Only numbers between 1-50.", [](const int& i) {return i <= 50; });
	opt.page_count = pages;

	cout << "What is the maximum amount of files wanted?" << endl;
	int d = check<int>("Invalid input, input only a numbers", "Your number is too large!", [](const int& i) { return i <= 10000; });
	opt.max_files = d;

	cout << "Do you want to gather all images when faced with an entire gallery? (y/n)" << endl;
	char all = check<char>("Invalid input, only input (y)es or (n)o","Only input (y)es or (n)o",yesOrNo );
	opt.all_gallery = all == 'y' ? true : false;

}


void runRedditDownloader(string& imgur_authorization,string& directory) {
	//Options dev_options{ 1,3000,"",false, imgur_authorization ,string("") ,CREATENEW,"New folder"};
	Options options{ 0,0,"",false,imgur_authorization,"",SKIP,"Your Pics" };
	bool new_dir = runMainOptions(options);
	redditOptions(options);
	RedditDownloader reddit_downloader(options);
	if (new_dir)
		reddit_downloader.options.current_path = reddit_downloader.createDirectory(directory);
	while (true) {
		string input;
		cout << "Enter a link to download, otherwise enter quit to exit." << endl;
		cin.ignore();
		getline(cin, input);
		if (input == "quit") {
			break;
		}
		else {
			vector<string> urls = reddit_downloader.getAllImages(input);
			if (urls.size() != 0) {
				cout << "You are downloading " << urls.size() << " files, do you want to continue (y/n)?" << endl;
				char amt = check<char>("Invalid input, (y) to continue, (n) to quit download; downloading : " + std::to_string(urls.size()) 
					+ " files","Only input (y)es or (n)o", yesOrNo);
				if (amt == 'y') {
					reddit_downloader.download(urls);
					cout << "--Work finished--" << endl;
				}
			}
			else {
				cout << "There was no media to download, make sure the link is a subreddit url and DOES contain media. \nurl recieved : " << input << endl;
			}
			cout << "Enter new options? (y/n)" << endl;
			char new_opts = check<char>("Invalid input, (y/n) for new options", "Only input (y)es or (n)o for new options",yesOrNo);
			if (new_opts == 'y') {
				redditOptions(reddit_downloader.options);
			}
		}

	}
}

#endif // !REDDITFUNCTIONS