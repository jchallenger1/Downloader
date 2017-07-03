#ifndef DOWNLOADER
#define DOWNLOADER
#include <vector>
#include <string>
#include <curl\curl.h>
#include </Json/json.hpp>

using std::string; using std::vector; using json = nlohmann::json;

enum class File {
		SKIP,OVERWRITE,CREATENEW };

struct Options {
	int page_count;//amount of pages will download
	unsigned int max_files; // maximum amount of posts; optional.
	string tag; //some posts will have a tag
	bool all_gallery; //imgur often has a gallery that can consist of 200+ imgs of possibly the same thing, option to turn this off.
	string imgur_auth;
	string current_path;//path where the application is located
	File duplicate_file; // if the file already exists, it tells us what we should do with it.
	string folder_name; //optional for the user to provide a name for the folder
};

class Downloader {
public:
	Downloader(const Options& player_options);
	virtual ~Downloader();
	Options options;
	virtual void getAllImages(string&) = 0;
	void download();
	string createDirectory(string) const; //creates a directory and modifies the folder_path to the input folder.
	virtual void websiteOptions(Options&) = 0; //options given by each website to adjust.
	vector<string> urls; //urls of one page
protected:
	/* VARIABLES */
	string folder_path;
	char error[CURL_ERROR_SIZE]; //message of error if one occurs
	CURL* curl; //curl object, responsible for interacting with servers via requests.
	json jsonp; //object that holds json from websites.
	bool all = false; // checks if the downloader should download all photos on imgur, or only one.

	/* MEMBER FUNCTIONS */
	void Downloader::initCurlSetup(const string & url, const string& buffer) noexcept;
	void createFolderPath(string&) const; //creates the string where all photos,gifs,vids are inputted into a folder named by the function.
	static bool removeNonSupported(const string&);//some posts will have images and post we cannot support, we must get rid of them.
	virtual vector<std::pair<string, string>> mapUrls(vector<string>&);
	virtual vector<string> getPureImgUrl(vector<std::pair<string,string>>&);//some posts will direct to a webpage with the img, but here we obtain the pure image file, not the webpage of it.
	void changeImgToMp4(string&); // changed imgur posts to mp4 extension, needed because .gif and .gifv can sometimes not work as expected.
	virtual void getUrlsFromJson(const string&, vector<string>&) = 0; //takes an unparsed json and gets the urls from it which points to the images or some random post we don't want.
	virtual bool validate(const string&) = 0; //validates the url to make sure it is a reddit/4chan/imgur url.
	void getFileNames(const vector<string>&,vector<std::pair<string, string>>&) const noexcept; //takes the file name off of the url.
};




class RedditDownloader : public Downloader {
public:
	using Downloader::Downloader;
	virtual void getAllImages(string&) override;
private:
	void nextPage(string&,string&) const;
	void appendJsonString(string&) const;
	virtual void getUrlsFromJson(const string&, vector<string>&) override;
	virtual bool validate(const string&) override;
	virtual void websiteOptions(Options&) override;
};



class ChanDownloader : public Downloader{
public:
	using Downloader::Downloader;
	virtual void getAllImages(string&) override;
private:
	string chan_sub;
	int min_size = 0;
	virtual void getUrlsFromJson(const string&, vector<string>&) override;
	virtual bool validate(const string&) override;
	virtual void websiteOptions(Options&) override;
	inline string getBaseUrl(const string&) const;
	void appendJsonString(string&) const;
	void getChanSub(const string&);
	vector<string> getThreads(const string&, const string&);
};



class TumblrDownloader : public Downloader {
public:
	using Downloader::Downloader;
	virtual void getAllImages(string&) override;
	virtual void websiteOptions(Options&) override;
private:
	string tumblr_auth = "";
	string pure_url; //the base url without any extensions, 'prefixes' or 'suffixes'.
	int min_size = 0;
	virtual void getUrlsFromJson(const string&, vector<string>&) override;
	virtual bool validate(const string&) override;
	void getPureUrl(const string&);
	template<typename T>
	bool hasCorrectTag(T&) const;
	
};

class ImgurDownloader : public Downloader {
public:
	using Downloader::Downloader;
	virtual void getAllImages(string&) override;
	virtual void websiteOptions(Options&) override;
	int min_size = 0;
private:
	string query_string;
	virtual void getUrlsFromJson(const string&, vector<string>&) override;
	virtual bool validate(const string&) override;
	void getQueryString(const string&);
};






#endif // !DOWNLOADER


