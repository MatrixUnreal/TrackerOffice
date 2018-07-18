#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <chrono>
#include <mutex>

#include "Util.hpp"

//#include "DBAccessor.hpp"
#include "/usr/include/curl/curl.h"


std::mutex logMutex;

void log(const std::string & msg)
{
	using namespace std::chrono;
	std::lock_guard<std::mutex> lock(logMutex);
	system_clock::time_point p = system_clock::now();
	std::time_t t = system_clock::to_time_t(p);
	std::clog << std::ctime(&t) << ": " << msg << std::endl;
}

/* Напоминалка о структуре
struct tm {
         tm_sec,                         // секунды 0..59 
         tm_min,                         // минуты  0..59 
         tm_hour,                        // час дня 0..23 
         tm_mday,                    // день месяца 1..31 
         tm_mon,                           // месяц 0..11 
         tm_year,                       // год после 1900 
         tm_wday,         // день недели, 0..6 (Sun..Sat) 
         tm_yday,                    // день года, 0..365 
         tm_isdst;        // >0, если есть поправка на сдвиг,
                          //   =0, если нет поправки,
                          //   <0, если неизвестно 
};
*/

std::map<std::string, std::string>
getParams(const std::string & filename,
	const std::string & field,
	const std::vector<std::string> & keys)
{
	std::map<std::string, std::string> params;
	std::ifstream inf(filename);
	if (!inf.is_open())
	{
		std::cout << "Error opening file.\n";
		return params;
	}
	
	std::string line;
	std::getline(inf, line);
	
	while (line.find(std::string("[" + field + "]")) == std::string::npos
		&& inf)
	{
		std::getline(inf, line);
	}
	std::vector<std::string> configStrings;
	std::getline(inf, line);
	while (!std::regex_search(line, std::regex(R"(^\[.*\]$)")) && inf)
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.erase(line.size() - 1, 1);
		}
		if (!line.empty() && line.front() != '#')
		{
			configStrings.push_back(line);
		}
		std::getline(inf, line);
	}
	std::for_each(keys.begin(), keys.end(), [&params, configStrings](std::string key)
	{
		for (auto str : configStrings)
		{
			if (str.substr(0, str.find('=')) == key)
			{
				params[key] = str.substr(str.find('=') + 1);
				break;
			}
		}
	});
	return params;
}

std::vector<std::pair<std::string, std::string>>
getParams(const std::string & filename,
	const std::string & field)
{
	std::vector<std::pair<std::string, std::string>> params;
	std::ifstream inf(filename);
	if (!inf.is_open())
	{
		return params;
	}
	
	std::string line;
	std::getline(inf, line);
	
	while (line.find(std::string("[" + field + "]")) == std::string::npos
		&& inf)
	{
		std::getline(inf, line);
	}
	std::vector<std::string> configStrings;
	std::getline(inf, line);
	while (!std::regex_search(line, std::regex(R"(^\[.*\]$)")) && inf)
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.erase(line.size() - 1, 1);
		}
		if (!line.empty() && line.front() != '#')
		{
			configStrings.push_back(line);
		}
		std::getline(inf, line);
	}
	for (auto str : configStrings)
	{
		if (str.find('=') == std::string::npos)
		{
			continue;
		}
		std::string key = str.substr(0, str.find('='));
		params.emplace_back(key, str.substr(str.find('=') + 1));
	}
	return params;
}

//TODO test this function
std::vector<std::string> split(const std::string & str, char ch, bool skipEmptyStrings)
{
	auto pos = str.find(ch);
	unsigned int initialPos = 0;
	std::vector<std::string> strings;

	// Decompose statement
	while (pos != std::string::npos)
	{
		if (!skipEmptyStrings || initialPos < pos)
		{
			strings.push_back(str.substr(initialPos, pos - initialPos));
		}

		initialPos = pos + 1;
		pos = str.find(ch, initialPos);
	}

	// Add the last one
	if (!skipEmptyStrings || initialPos < str.size())
	{
		strings.push_back(str.substr(initialPos));
	}

	return strings;
}

bool isSameImage(const cv::Mat& img1, const cv::Mat& img2)
{
	if (img1.size() != img2.size() ||
		img1.type() != img2.type())
	{
		return false;
	}

	cv::Mat res;
	cv::absdiff(img1, img2, res);
	int nonZero = cv::countNonZero(res);
	return nonZero < img1.total() * 0.02; //less than 2% difference
}

bool similar(cv::Rect left, cv::Rect right, cv::Point margin)
{
	cv::Rect outboundRect(left.x - margin.x, left.y - margin.y,
		left.width + margin.x * 2, left.height + margin.y * 2);
	cv::Rect inboundRect(left.x + margin.x, left.y + margin.y,
		left.width - margin.x * 2, left.height - margin.y * 2);

	if ((outboundRect & right) == right &&
		(inboundRect & right) == inboundRect)
	{
		return true;
	}
	return false;
}

bool similar(bbox_t left, bbox_t right, cv::Point margin)
{
	return similar(boxToRect(left), boxToRect(right), margin);
}

cv::Point shiftAndScale(cv::Point point, cv::Point shift, double scaleFactor)
{
	point += shift;
	point *= scaleFactor;
	return point;
}

cv::Rect shiftAndScale(cv::Rect rect, cv::Point shift, double scaleFactor)
{
	rect += shift;
	rect.x *= scaleFactor;
	rect.y *= scaleFactor;
	rect.width *= scaleFactor;
	rect.height *= scaleFactor;
	return rect;
}

cv::Size scale(cv::Size size, double scaleFactor)
{
	size.width *= scaleFactor;
	size.height *= scaleFactor;
	return size;
}

cv::Size scale(cv::Size size, double xScaleFactor, double yScaleFactor)
{
	size.width *= xScaleFactor;
	size.height *= yScaleFactor;
	return size;
}

bbox_t scale(bbox_t box, double xScaleFactor, double yScaleFactor)
{
	box.x *= xScaleFactor;
	box.w *= xScaleFactor;
	box.y *= yScaleFactor;
	box.h *= yScaleFactor;
	return box;
}

cv::Rect scale(cv::Rect rect, double xScaleFactor, double yScaleFactor)
{
	rect.x *= xScaleFactor;
	rect.width *= xScaleFactor;
	rect.y *= yScaleFactor;
	rect.height *= yScaleFactor;
	return rect;
}

cv::Size scaleTo(cv::Size size, cv::Size outBound)
{
	double xScale = static_cast<float>(outBound.width) / size.width;
	double yScale = static_cast<float>(outBound.height) / size.height;
	return scale(size, std::min(xScale, yScale));
}

cv::Rect adjustROI(cv::Rect rect, cv::Size size)
{
	auto res = rect;
	res.x = std::max(res.x, 0);
	res.y = std::max(res.y, 0);
	res.x = std::min(res.x, size.width);
	res.y = std::min(res.y, size.height);
	res.width = std::min(size.width - res.x, res.width);
	res.height = std::min(size.height - res.y, res.height);
	return res;
}

cv::Mat getSafeRoi(const cv::Mat& img, cv::Rect roi)
{
	roi = roi & cv::Rect(0, 0, img.size().width, img.size().height);

	return img(roi);
}

cv::Mat resizeIfBigger(const cv::Mat& image, cv::Size size)
{
	cv::Mat retImg;
	if (image.empty())
	{
		return retImg;
	}

	if (image.cols > size.width ||
		image.rows > size.height)
	{
		float xScale = static_cast<float>(size.width) / image.cols;
		float yScale = static_cast<float>(size.height) / image.rows;
		float scale = std::min(xScale, yScale);
		cv::resize(image, retImg, cv::Size(image.cols * scale, image.rows * scale));
	}
	else
	{
		retImg = image.clone();
	}
	return retImg;
}

cv::Size getSize(bbox_t box)
{
	return cv::Size(box.w, box.h);
}

cv::Size getSize(cv::Rect rect)
{
	return rect.size();
}

int msleep(unsigned long milisec)  
{  
 
    struct timespec req={0};  
 
    time_t sec=(int)(milisec/1000);  
 
    milisec=milisec-(sec*1000);  
 
    req.tv_sec=sec;  
 
    req.tv_nsec=milisec*1000000L;  
 
    while(nanosleep(&req,&req) == -1)
        continue;  
 
    return 1;  
 
}  

bool insert(int doornum, std::string input, std::string cabnum)
{
	/*DBAccessor::Config cfg = { "localhost","root","root","intersection" };
	DBAccessor db(cfg); 
	std::string s = "INSERT INTO door(doornum, status, cab) VALUES(";
	s += std::to_string(doornum);
	s += ", \"";
	s += input;
	s += "\", \"";
	s += cabnum + "\");";

	return db.execDirect(s);
	*/
	return 0;
}

long insert_Entry(std::string name, std::string ev_type, std::string cam, std::string cab)
{
	/*
	DBAccessor::Config cfg = { "localhost","root","root","intersection" };
	DBAccessor dba(cfg);
	DBAccessor::ParamWhere paramName = { "name", name, "=", true };
	DBAccessor::ParamWhere paramType = { "entry", ev_type, "=", true };
    DBAccessor::ParamWhere paramTime = { "time_event", "DATE_SUB(now(), INTERVAL 5 SECOND)", ">=", false };
    std::vector<DBAccessor::ParamWhere> params = { paramName, paramTime };
    if (!dba.getColumnValueByParams("Entry", "id", params, "id", true).empty())
    {
		std::string s = "INSERT INTO Entry(name, event, cam, cab) VALUES(\"";
		s += name;
		s += "\", \"";
		s += ev_type;
		s += "\", \"";
		s += cam;
		s += "\", \"";
		s += cab + "\");";
		std::cout<<s<<'\n';
		return dba.execDirect(s);
	}
	else */return 0;
}

const std::vector<std::string> explode(const std::string& s, const char& c)
{
	std::string buff{""};
	std::vector<std::string> v;
	
	for(auto n:s)
	{
		if(n != c) buff+=n; else
		if(n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if(buff != "") v.push_back(buff);
	
	return v;
}

void error(const char *msg)
{
    perror(msg);
}

int OpenDoor_system()
{
	system("curl --user admin:startup123 -T \"open_door.xml\" -X PUT http://192.168.90.170/ISAPI/VideoIntercom/remoteOpenDoor");
	return 0;
}


//функция обратного вызова
int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
{
  //переменная - результат, по умолчанию нулевая
  int result = 0;
  //проверяем буфер
  if (buffer != NULL)
  {
    //добавляем к буферу строки из data, в количестве nmemb
    buffer->append(data, size * nmemb);
    //вычисляем объем принятых данных
    result = size * nmemb;
  }
  //возвращаем результат
  return result;
}

std::vector<std::string> ReadHeader(const char *input_file_name)
{
	std::ifstream in(input_file_name);
	std::string line;
	std::vector<std::string> section;
	while (getline(in, line))
	{
		std::istringstream iss(line);
		if (iss.get() == 91)
		{
			line=line.substr(1, line.size() - 2);
			section.push_back(line);
		}
	}
	in.close();
	return section;
}

void redirect_cout()
{
	freopen( "cout.html", "w", stdout );
}

void redirect_cerr()
{
	freopen( "error.log", "w", stderr );
	std::cerr << "Error message" << std::endl;
}

std::vector<cv::Rect> BoxToRect(std::vector<bbox_t> v_input)
{
	std::vector<cv::Rect> v_output;	
	for (auto input : v_input)
	{
		cv::Rect objectRect(input.x, input.y, input.w, input.h);
		v_output.push_back(objectRect);		
	}	
	return v_output;
}

cv::Rect boxToRect(bbox_t box)
{
	return cv::Rect(box.x, box.y, box.w, box.h);
}

void Safe_dimensions(cv::Rect & changeable_rect,cv::Rect original_rect,cv::Mat frame,int _x,int _y,int _w,int _h)
{
	auto rect = original_rect;
	rect.x -= _x;
	rect.y -= _y;
	rect.width += _w + _x;
	rect.height += _h + _y;

	rect.x = std::max(rect.x, 0);
	rect.width = std::min(frame.cols - rect.x, rect.width);
	rect.y = std::max(rect.y, 0);
	rect.height = std::min(frame.rows - rect.y, rect.height);

	changeable_rect = rect;
}