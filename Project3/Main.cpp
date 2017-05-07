#include <string>
#include <iostream>
#include <iomanip>
#include "Download.h"
#include "Functions.h"
using std::string;  using std::endl; using std::cout; 

int main(int argc, char ** argv) {
#if defined(WIN32) || defined(_WIN32) ||defined(__WIN32) || defined(_WIN64)
	string your_client_id("");
	string imgur_authorization("Authorization: Client-ID " + your_client_id);
	string current_dir(argv[0]);
	//Options dev_options{ 1,3000,"",false, imgur_authorization ,string("") ,CREATENEW,"New folder"};
	Options options{ 0,0,"",false,imgur_authorization,"",SKIP,"Your Pics" };
	cout << "Choose a downloader, (reddit),(imgur),(4chan),(tumblr)" << endl;
	string type = check<string>("Invalid input, (reddit), (imgur), (4chan), or (tumblr)","", [](const string& s){
		return s == "reddit" || s == "imgur" || s == "4chan" || s == "tumblr"; });
	if (type == "reddit") {
		RedditDownloader reddit_downloader(options);
		runMainProgram(current_dir,reddit_downloader);
	}
	else if (type == "4chan") {
		ChanDownloader chan_downloader(options);
		runMainProgram(current_dir, chan_downloader);
	}
	else {
		cout << "Others are not supported (yet), sorry! " << endl;
	}
	system("pause");
	return 0;
#else
	std::cerr << "Sorry your OS is not supported!\n Press Enter to exit." << endl;
	std::cin.get();
	return 0;
#endif
}
