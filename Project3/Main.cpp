#include <string>
#include <iostream>
#include "Download.h"
#include "Functions.h"
#include <vector>
using std::string; using std::vector; using std::endl; using std::cout; using std::ofstream; using std::cin;

int main(int argc, char ** argv) {
#if defined(WIN32) || defined(_WIN32) ||defined(__WIN32) || defined(_WIN64)
	string your_client_id("");
	string imgur_authorization("Authorization: Client-ID " + your_client_id);
	string current_dir(argv[0]);

	cout << "Choose a downloader, (reddit),(imgur),(4chan),(tumblr)" << endl;
	string type = check<string>("Invalid input, (reddit), (imgur), (4chan), or (tumblr)", {"reddit","imgur","4chan","tumblr"});
	if (type == "reddit") {
		runRedditDownloader(imgur_authorization, current_dir);
	}
	else if (type == "4chan") {
		runChanDownloader(current_dir);
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
