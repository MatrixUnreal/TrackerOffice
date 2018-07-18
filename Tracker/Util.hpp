#pragma once
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"

#include "yolo_v2_class.hpp"

const char * const CAM_CONFIG_NAME = "data/config.cfg";
const char * const DOOR_CONFIG_NAME = "data/door.cfg";

const std::vector<std::string> unrecognized =
	{ "incorr_sz","bigger","some","NO_FACE","UNRECOGNIZED", "",
	"Incorr_img","Incorr_img1","Incorr_img2","BLURRED","NO_ENCODINGS" };

struct Schedule
{
	std::string start;	
	std::string end;
	std::vector<int> dayoffs;
};


class LogStreamGuard
{
public:
	LogStreamGuard(const std::string & filename, std::ios_base::openmode m) : f(filename)
	{
		buf = std::clog.rdbuf(f.rdbuf());
	}
	~LogStreamGuard()
	{
		std::clog.rdbuf(buf);
		f.close();
	}

	void check() const
	{
		if (!f.is_open())
		{
			std::cerr << "Problem opening log file.\n"
				<< "No logging during this run.\n";
		}
		else
		{
			std::cout << "Log file opened.\n";
		}
	}

private:
	std::ofstream f;
	std::streambuf * buf;
};

void log(const std::string & msg);

std::vector<std::string> split(const std::string & str, char ch, bool skipEmptyStrings = true);

std::map<std::string, std::string>
getParams(const std::string & filename,
	const std::string & field,
	const std::vector<std::string> & keys);
std::vector<std::pair<std::string, std::string>>
getParams(const std::string & filename,
	const std::string & field);

bool isSameImage(const cv::Mat& img1, const cv::Mat& img2);
//checks if right is similar to left
bool similar(cv::Rect left, cv::Rect right, cv::Point margin);
bool similar(bbox_t left, bbox_t right, cv::Point margin);
cv::Point shiftAndScale(cv::Point point, cv::Point shift, double scaleFactor);
cv::Rect shiftAndScale(cv::Rect rect, cv::Point shift, double scaleFactor);
cv::Size scale(cv::Size size, double scaleFactor);
cv::Size scale(cv::Size size, double xScaleFactor, double yScaleFactor);
bbox_t scale(bbox_t box, double xScaleFactor, double yScaleFactor);
cv::Rect scale(cv::Rect rect, double xScaleFactor, double yScaleFactor);
cv::Size scaleTo(cv::Size size, cv::Size outBound);
cv::Rect adjustROI(cv::Rect rect, cv::Size size);
cv::Mat getSafeRoi(const cv::Mat& img, cv::Rect roi);
cv::Mat resizeIfBigger(const cv::Mat& image, cv::Size size); //deep copy inside
cv::Size getSize(bbox_t box);
cv::Size getSize(cv::Rect rect);

template<typename T>
void joinOverlapped(std::vector<T>& rects)
{
	for (auto detectIt = rects.begin();
		detectIt != rects.end() && detectIt + 1 != rects.end();
		detectIt++)
	{
		auto badDetectIt = std::find_if(detectIt + 1, rects.end(),
			[detectIt] (T rect)
			{
				return similar(*detectIt, rect, getSize(*detectIt) / 6);
			});
		while (badDetectIt != rects.end())
		{
			auto it = rects.erase(badDetectIt);
			badDetectIt = std::find_if(it, rects.end(),
				[detectIt] (T rect)
				{
					return similar(*detectIt, rect, getSize(*detectIt) / 6);
				});
		}
	}
}

int writer(char *data, size_t size, size_t nmemb, std::string *buffer);
int OpenDoor_system();

int msleep(unsigned long milisec);
bool insert(int doornum, std::string status, std::string cabnum);
long insert_Entry(std::string name, std::string ev_type, std::string cam, std::string cab);
const std::vector<std::string> explode(const std::string& s, const char& c);
std::vector<std::string> ReadHeader(const char *input_file_name);
void redirect_cout();
void redirect_cerr();
std::vector<cv::Rect>  BoxToRect(std::vector<bbox_t> v_input);
cv::Rect boxToRect(bbox_t box);
void Safe_dimensions(cv::Rect & changeable_rect,cv::Rect original_rect,cv::Mat,int _x,int _y,int _w,int _h);

template <typename T>
void any_clear(T &val)
{
	T tmp;
	std::swap(tmp, val);
}