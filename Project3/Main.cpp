#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <curl\curl.h>
#include "Download.h"
#include "Functions.h"
using std::string;  using std::endl; using std::cout; 


int main(int argc, char ** argv) {
#if defined(WIN32) || defined(_WIN32) ||defined(__WIN32) || defined(_WIN64)
	string your_client_id("");
	string imgur_authorization("Authorization: Client-ID " + your_client_id);
	string current_dir(argv[0]);
	Options options{ 0,0,"",false,imgur_authorization,"",File::SKIP,"Your Pics" };
	cout << "Choose a downloader, (reddit),(imgur),(4chan),(tumblr)" << endl;

	string type = check<string>("Invalid input, (reddit), (imgur), (4chan), or (tumblr)","", [](const string& s){
		return s == "reddit" || s == "imgur" || s == "4chan" || s == "tumblr";
	});
	if (type == "reddit") {
		RedditDownloader reddit_downloader(options);
		runMainProgram(current_dir,reddit_downloader);
	}
	else if (type == "4chan") {
		cout << "Note : getting images from multiple threads can take a while, 2 seconds between each thread." << endl;
		ChanDownloader chan_downloader(options);
		runMainProgram(current_dir, chan_downloader);
	}
	else if (type == "tumblr") {
		TumblrDownloader tumblr_downloader(options);
		runMainProgram(current_dir, tumblr_downloader);
	}
	else if (type == "imgur") {
		ImgurDownloader imgur_downloader(options);
		runMainProgram(current_dir, imgur_downloader);
	}
	cout << "Program exiting in 30 seconds" << endl;
	std::this_thread::sleep_for(std::chrono::seconds(30));
	return 0;
#else
	std::cerr << "Sorry your OS is not supported!\n Press Enter to exit." << endl;
	std::cin.get();
	return 0;
#endif
}
