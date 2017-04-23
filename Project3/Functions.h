#ifndef FUNCTIONS
#define FUNCTIONS
#include <string>
#include <initializer_list>
#include <iostream>
#include "Download.h"
using std::string; using std::cin; using std::cout;
template<typename T>
T check(const string& message, const std::initializer_list<T>& criteria) {//checking inputs recieved, if the stream is good and if they match what characters were wanted.
	T x;
	while (cin.good()) {
		cin >> x;
		if (!cin.good()) {
			cout << message << endl;
			cin.clear();
			cin.ignore();
			continue;
		}
		else {//cin is in a good condition
			bool good = false;
			for (auto begin = criteria.begin(); begin != criteria.end(); begin++) {//now we check if user's input matches any criteria.
				if (*begin == x) {
					good = true;
				}
			}//for
			if (good) {
				return x;
			}
			else {
				cout << message << endl;
				continue;
			}
		}

	}//while
}

template<typename T>
T vectorCheck(const string& message, const std::vector<T>& criteria) {//checking inputs recieved, if the stream is good and if they match what characters were wanted.
	T x;
	while (cin.good()) {
		cin >> x;
		if (!cin.good()) {
			cout << message << endl;
			cin.clear();
			cin.ignore();
			continue;
		}
		else {//cin is in a good condition
			bool good = false;
			auto iter = std::find(criteria.begin(), criteria.end(), x);

			if (iter != criteria.end()) {
				return x;
			}
			else {
				cout << message << endl;
				continue;
			}
		}

	}//while
	assert(cin.good() >=1);
}

extern int maximumCheck(const int&);

extern size_t writeJsonData(void *buffer, size_t size, size_t nmemb, void *userp);

extern size_t downloadFile(void *buffer, size_t size, size_t nmemb, std::ofstream* userp);

extern int getFileSize(const string&);


#endif // !FUNCTIONS

