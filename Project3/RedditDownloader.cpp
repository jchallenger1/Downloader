#ifndef REDDITDOWNLOADER
#define REDDITDOWNLOADER
#include <string>
#include <iostream>
#include <curl/curl.h>
#include <regex>
#include "Download.h"
#include "Functions.h"
using std::cin; using std::cout; using std::endl;

string RedditDownloader::getUrlsFromJson(string& data) {
	jsonp = json::parse(data.c_str());
	auto n = jsonp["data"]["children"];
	auto begin = n.begin();
	auto end = n.end();
	while (begin != end) {
		urls.push_back((*begin)["data"]["url"]);
		begin++;
	}
	return "";
}

void RedditDownloader::nextPage(string& query_string,string& url) {
	auto last_query = std::find(url.rbegin(), url.rend(), '?');
	if (last_query != url.rend())
		url.erase(last_query.base() - 1, url.end());
	url.append("?after=" + query_string);
	url.append("");
}
/*
* We only support pure reddit post,Imgur, 4chan, tumblr, gfycat posts.
* This function takes the url and checks if it is suitable for usage.
* We do not support whole reddit posts, whole chan posts, or whole tumblr posts.
* We do support imgur gallerys, implementation in another function.
*/


vector<string> RedditDownloader::getAllImages(string& url) {
	vector<string> image_urls;
	if (validate(url)) {
		url = url[url.size() - 1] == '/' ? url : url += "/";
		url.append(".json");
		while (options.page_count > 0 && image_urls.size() < options.max_files) {
			string all_reddit_json;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeJsonData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &all_reddit_json);
			auto response = curl_easy_perform(curl);
			curl_easy_reset(curl);
			if (response == CURLE_OK && !all_reddit_json.empty()) {
				getUrlsFromJson(all_reddit_json); // parsing the json and getting the urls.
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
			if (jsonp["data"]["after"].is_null()) {
				break;
			}
			else {
				string next_page_query = jsonp["data"]["after"];
				nextPage(next_page_query,url);
			}
			options.page_count--;
		} // !while
	} // !validate if
	else {
		cout << url << "\n is not a valid url." << endl;
	}
	if (image_urls.size() > options.max_files) {
		image_urls = vector<string>(image_urls.begin(), image_urls.begin() + options.max_files);
	}

	return image_urls;
}

bool RedditDownloader::validate(string& url) {
	bool a = false;
	try {
		string pattern("https?://www.reddit.com/r/\\w+(/(?!comments))?");
		std::regex reg(pattern);
		a = std::regex_search(url, reg);
	}
	catch (std::regex_error err) {
		cout << err.what() << "\nline:" << err.code() << endl;
	}
	
	return a;
}



#endif // !REDDITDOWNLOADER


