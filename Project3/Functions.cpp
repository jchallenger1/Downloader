#include "Functions.h"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
using std::string; using std::cout; using std::endl; using std::cin;

size_t writeJsonData(void *buffer, size_t size, size_t nmemb, void *userp) {//converts a buffer to a string via userp*
	static_cast<string*>(userp)->append(static_cast<char*>(buffer), size* nmemb);
	return size * nmemb;
}

size_t downloadFile(void *buffer, size_t size, size_t nmemb, std::ofstream* userp) {//Takes a buffer and downloads the file
	userp->write(static_cast<char*>(buffer), size*nmemb);
	return size*nmemb;
}

int getFileSize(const std::string &fileName) {
	std::ifstream file(fileName.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open()) {
		return -1;
	}

	file.seekg(0, std::ios::end);
	int fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	file.close();

	return fileSize;
}

int maximumCheck(const int& g) {
	int x;
	while (cin.good()) {
		cin >> x;
		if (!cin.good()) {
			cout << "You entered either too large a number or not a number, enter max amount of pages." << endl;
			cin.clear();
			cin.ignore();
			continue;
		}
		else {//cin is in a good condition
			if (g > x) {
				return x;
			}
			else {
				continue;
			}
		}

	}//while
}

bool redditOptions(Options& opt) {
	bool create_new = false;
	cout << "How many pages do you want? Max of 50." << endl;
	vector<int> ints;
	for (int x = 1; x != 51; x++)
		ints.push_back(x);
	int pages = vectorCheck("Invalid input enter amt of pages 1-50.", ints );
	opt.page_count = pages;
	cout << "What is the maximum amount of files wanted?" << endl;
	int d = maximumCheck(10000);
	opt.max_files = d;
	opt.tag = ""; //reddit doesn't use tags.
	cout << "Do you want to gather all images when faced with an entire gallery? (y/n)" << endl;
	char all = check("Invalid input, Gather all images?(y/n)", { 'y','n' });
	opt.all_gallery = all == 'y' ? true : false;
	cout << "These options is what happens when a duplicate file occurs,\nType 1 to skip it, 2 to overwrite it, 3 to create a new one. " << endl;
	int dup = check("Invalid input,(1) to skip,(2) to overwrite,(3) to create new", { 1,2,3 });
	opt.duplicate_file = dup == 1 ? SKIP : dup == 2 ? OVERWRITE : CREATENEW;
	cout << "Enter (y) if you want to enter a path to a folder to download images, enter (n) if you want us to create a new folder for you." << endl;
	char path = check("Invalid input, (y) to enter path, (n) for us making a new folder", { 'y','n' });
	string download_path;
	if (path == 'y') {
		while (true) {
			cout << "Enter your path " << endl;
			cin.ignore();
			getline(cin, download_path);
			std::experimental::filesystem::path user_path(download_path);
			bool exists = std::experimental::filesystem::exists(user_path);
			if (!exists) {
				cout << download_path << " is not a valid directory." << endl;
				cout << "Do you want to try again or allow the program to make a new folder? (y/n)" << endl;
				char again = check("Invalid input (y/n)", { 'y','n' });
				if (again == 'y') {
					download_path.clear();
					continue;
				}
				else {
					create_new = true;
					break;
				}
					
			} // !if exists
			else {
				opt.current_path = download_path;
				break;
			}
		}// !while
		
	} // !if
	else {
		create_new = true;
	}
	return create_new;
}



/* DOWNLOADER RUNNERS */
void runRedditDownloader(string& imgur_authorization,string& directory) {
	//Options dev_options{ 2,3000,"",false, imgur_authorization ,string(argv[0] + "\\New Folder") ,SKIP,"New folder"};
	Options options{ 0,0,"",false,imgur_authorization,"",SKIP,"Your Pics" };
	bool new_dir;
	new_dir = redditOptions(options);
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
			cout << "You are downloading " << urls.size() << " files, do you want to continue (y/n)?" << endl;
			char amt = check("Invalid input, (y) to continue, (n) to quit download.", { 'y','n' });
			if (amt == 'y') {
				reddit_downloader.download(urls);
				cout << "--Work finished--" << endl;
			}
			cout << "Enter new options? (y/n)" << endl;
			char new_opts = check("Invalid input, (y/n) for new options", { 'y','n' });
			if (new_opts == 'y') {
				new_dir = redditOptions(reddit_downloader.options);
			}
		}

	}
}