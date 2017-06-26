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

long long getFileSize(const std::string &fileName) {
	std::ifstream file(fileName.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open()) {
		return -1;
	}

	file.seekg(0, std::ios::end);
	long long fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	file.close();
	return fileSize;
}

std::function<bool(const char&)> yesOrNo = [](const char& user_input) -> bool {
	return user_input == 'y' || user_input == 'n';
};

bool runMainOptions(Options& opt) {
	bool create_new = false;// if it should create a new folder

	cout << "These options is what happens when a duplicate file occurs,\nType 1 to skip it, 2 to overwrite it, 3 to create a new one. " << endl;
	int dup = check<int>("Invalid input,(1) to skip,(2) to overwrite,(3) to create new", "Only input 1 2 or 3.", [](const int& i) {return i >= 1 && i <= 3; });
	opt.duplicate_file = dup == 1 ? SKIP : dup == 2 ? OVERWRITE : CREATENEW;

	cout << "Enter (y) if you want to enter a path to a folder to download images, enter (n) if you want us to create a new folder for you." << endl;
	char path = check<char>("Invalid input, (y) to enter path, (n) for us making a new folder", "Only enter (y)es or (n)o", yesOrNo);
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
				char again = check<char>("Invalid input, (y) to enter path, (n) for us making a new folder", "Only enter (y)es or (n)o", yesOrNo);

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

void runMainProgram(const string& directory, Downloader& website_downloader) {//website_downloader's options must have near nothing!
	bool new_dir = runMainOptions(website_downloader.options);//false;
	website_downloader.websiteOptions(website_downloader.options);
	if (new_dir)
		website_downloader.options.current_path = website_downloader.createDirectory(directory);
	while (true) {
		string input;
		cout << "Enter a link to download, otherwise enter quit to exit." << endl;
		cin.ignore();
		getline(cin, input);
		if (input == "quit") {
			break;
		}
		else {
			vector<string> urls = website_downloader.getAllImages(input);
			if (urls.size() != 0) {
				cout << "You are downloading " << urls.size() << " files, do you want to continue (y/n)?" << endl;
				char amt = check<char>("Invalid input, (y) to continue, (n) to quit download; downloading : " + std::to_string(urls.size())
					+ " files", "Only input (y)es or (n)o", yesOrNo);
				if (amt == 'y') {
					website_downloader.download(urls);
					cout << "--Work finished--" << endl;
				}
				else {
					cout << "Do you want to reduce amount of files downloaded (y), or quit completely(n)?" << endl;
					char reduce = check<char>("Invalid input, (y) to reduce, (n) to quit", "Enter (y/n)", yesOrNo);
					if (reduce == 'y') {
						cout << "Enter amount of files, must be less than the original amount." << endl;
						int file_count = check<int>("Input only a number.", "The number must be less than" + std::to_string(urls.size()),
							[&urls](const int& i) ->bool { return i < static_cast<int>(urls.size()); });
						urls.resize(file_count);
						website_downloader.download(urls);
					}
				}
			}
			else {
				cout << "There was no media to download, make sure the link is a subreddit url and DOES contain media. \nurl recieved : " << input << endl;
			}
			cout << "Enter new options? (y/n)" << endl;
			char new_opts = check<char>("Invalid input, (y/n) for new options", "Only input (y)es or (n)o for new options", yesOrNo);
			if (new_opts == 'y') {
				website_downloader.websiteOptions(website_downloader.options);
			}
		}

	}
}