#pragma once
#include <string>
#include <vector>
#include <map>

#include <opencv2/opencv.hpp>

#include "mymath.h"
#include "PersonDetect.h"


//distance to line in pixels in which intersection is not counted
const size_t LINE_BUFFER_DISTANCE = 25;

struct Intersection
{
	enum Direction { Left, Right };
	
	std::vector<cv::Point> polygon;
	std::string polygonID;
	Direction direction;
	Segment s;
	time_t timeOfFirstPoint;
};

struct IntersectionData
{
	std::map<std::string, std::vector<cv::Point>> polygons;
	std::vector<cv::Point> oldPoints;
	cv::Point newPoint;
};

struct LastPointOutOfPolygonBufferInfo
{
	bool refreshNeeded;
	time_t time;
};


struct Event
{
	std::string name;
	std::vector<std::vector<std::pair<std::string, Intersection::Direction>>> waysToComplete;

	Event(const std::string& _name,
		std::vector<std::vector<std::pair<std::string,
		Intersection::Direction>>>&& _waysToComplete)
		:
		name(_name), waysToComplete(std::forward<decltype(waysToComplete)>(_waysToComplete))
	{}
};


class detectedEvent
{	
public:
	detectedEvent(){};
	detectedEvent(std::string _name,std::string _type,std::string _cab,std::string _cam,std::vector<PersonDetect> _detects, time_t _ev_time)
	:name(_name), type(_type), cab(_cab),cam(_cam), detects(_detects), ev_time(_ev_time)
	{}
	~detectedEvent()
	{}
	void print()
	{
		std::cout<<name<<" has "<<type<<" from/in "<<cab<<" at "<<ev_time<<'\n';
	}

	std::string getTimestamp() const
	{
		const int BUFF_LEN = 25;
		char buff[BUFF_LEN];
		strftime(buff, BUFF_LEN, "%Y-%m-%d %H:%M:%S", localtime(&ev_time));
		return buff;
	}

	int id;
	std::string name;
	std::string type;
	std::string cab;
	std::string cam;
	//std::string video;
	time_t ev_time;
	std::vector<PersonDetect> detects;
};