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
