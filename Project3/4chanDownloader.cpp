#ifndef CHAN
#define CHAN

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include </Json/json.hpp>
#include <curl/curl.h>
#include "Download.h"

using std::string; using std::cin; using std::cout; using std::endl;

vector<string> ChanDownloader::getAllImages(string& url) {
	if (validate(url)) {

	}
	else {
		cout << url << " is not a valid url." << endl;
	}
	return vector<string>();
}

bool ChanDownloader::validate(string& url) {
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

void runChanDownloader(string& current_dir) {

}

#endif