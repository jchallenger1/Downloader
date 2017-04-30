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