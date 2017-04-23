#ifndef DOWNLOADER
#define DOWNLOADER
#include <vector>
#include <string>
#include <curl\curl.h>
#include </Json/json.hpp>

using std::string; using std::vector; using json = nlohmann::json;

enum DuplicateFile {
		SKIP,OVERWRITE,CREATENEW };

struct Options {
	int page_count;//amount of pages will download
	int max_files; // maximum amount of posts; optional.
	string tag; //some posts will have a tag
	bool all_gallery; //imgur often has a gallery that can consist of 200+ imgs of possibly the same thing, option to turn this off.
	string imgur_auth;
	string current_path;//path where the application is located
	int duplicate_file; // if the file already exists, it tells us what we should do with it.
	string folder_name; //optional for the user to provide a name for the folder
};

class Downloader {
public:
	Downloader(const Options& player_options);
	virtual ~Downloader();
	Options options;
	virtual vector<string> getAllImages(string&) = 0;
	void download(vector<string>&);
	string createDirectory(string); //creates a directory and modifies the folder_path to the input folder.
protected:
	/* VARIABLES */
	string folder_path;
	vector<string> urls; //urls of one page
	char error[CURL_ERROR_SIZE]; //message of error if one occurs
	CURL* curl; //curl object, responsible for interacting with servers via requests.
	json jsonp; //object that holds json from websites.
	bool all = false; // checks if the downloader should download all photos on imgur, or only one.

	/* MEMBER FUNCTIONS */
	void createFolderPath(string&); //creates the string where all photos,gifs,vids are inputted into a folder named by the function.
	static bool removeNonSupported(const string&);//some posts will have images and post we cannot support, we must get rid of them.
	virtual vector<std::pair<string, string>> mapUrls(vector<string>&);
	virtual vector<string> getPureImgUrl(vector<std::pair<string,string>>&);//some posts will direct to a webpage with the img, but here we obtain the pure image file, not the webpage of it.
	void changeImgToMp4(string&); // changed imgur posts to mp4 extension, needed because .gif and .gifv can sometimes not work as expected.
	virtual int getUrlsFromJson(string&) = 0; //takes an unparsed json and gets the urls from it which points to the images or some random post we don't want.
	virtual bool validate(string&) = 0; //validates the url to make sure it is a reddit/4chan/imgur url.
};




class RedditDownloader : public Downloader {
public:
	using Downloader::Downloader;
	virtual vector<string> getAllImages(string&) override;
private:
	void nextPage(string&,string&);
	void appendJsonString(string&);
	virtual int getUrlsFromJson(string&) override;
	virtual bool validate(string&) override;
};
extern bool redditOptions(Options&);

extern void runRedditDownloader(string& imgur_auth, string& curr_direct);





class ChanDownloader : public Downloader{
public:
	using Downloader::Downloader;
	virtual vector<string> getAllImages(string&) override;
private:
	virtual bool validate(string&) override;
};
extern void runChanDownloader(string&);



class ImgurDownloader : public Downloader {
public:
	
private:
	
};




class TumblrDownloader : public Downloader {
public:
private:
};

#endif // !DOWNLOADER
