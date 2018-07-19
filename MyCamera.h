#pragma once
#include "opencv2/opencv.hpp"

#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <stdio.h>
#include <chrono>
#include <thread>

#include "Video.h"
//#include "DBAccessor.hpp"

std::map<std::string, std::string> getParamsNew(const std::string & filename,const std::string & field,const std::vector<std::string> & keys);
std::vector<std::string> splitNew(const std::string & str, char ch, bool skipEmptyStrings);

class Camera
{
public:
	const int maxDurationVideo=40;
	cv::Mat frame,lastframe;
	struct LineOne
	{
		cv::Point p1;
		cv::Point p2;
		LineOne();
		LineOne(cv::Point p1,cv::Point p2)
		{
			this->p1=p1;this->p2=p2;
		}
	};

	struct Config
	{
		std::string camId;
		std::string url;
		std::string office;
		int sensitivity;
		int postwrite;
		int fps;
		cv::Rect detectionRoi;
		int rotate;
		std::string door;

		void parseParams(const std::string& camNumber);
	};
	Camera(){
		Video::createDateVideoFolder();
	};
	void initCamera();
	Camera(cv::String path);
	~Camera();

	bool getFrame();
	bool move_detect(int sensitivity);
	const cv::String currentDateTime();
	int getDuration();
	int setDuration(int input);
	void startWrite();
	void stopWrite();
	void setPath(cv::String input);
	cv::String getPath();
	void setNameVideoCamName(cv::String input);
	void setNameVideoCab(cv::String input);
	cv::String getNameVideoCamName();
	cv::String getNameVideoCab();
	int duration=0;	
	void setDrawRectangles(std::vector<cv::Rect> input );
	void setDrawLine(cv::Point p1,cv::Point p2);
	void setConfig(Config input);
	void setTimer();
	bool isTimeUp();
	void resetTimer();


private:	
	int maxBadFrame=5;
	std::vector<cv::Rect> rects;
	std::vector<LineOne> lines;
	bool canWrite = false;
	cv::String path;
	cv::String nameVideoFolder="Video/";
	cv::String nameVideoCab="";
	cv::String nameVideoCamName="";
	cv::String nameVideoFirstPart="";
	cv::String nameVideoExtention=".avi";
	cv::VideoCapture vcap;
	int frame_width, frame_height,frame_rate;
	cv::VideoWriter video;	
	cv::String pathWriter;
	Config config;
	std::time_t  timer=time(0);
	int currentCountOfTimer=0;
};
