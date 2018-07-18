#include "opencv2/opencv.hpp"
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#include <mutex>

#include "MyCamera.h"
#include "Tracker/track.h"
#include "Tracker/Ctracker.h"
#include "Tracker/Tuner.h"
#include "Tracker/ThreadSafeDetector.h"
#include "Setting.h"


using namespace std;
using namespace cv;

int maxDuration=80;	
/*
std::vector<cv::Rect> getFaces(const cv::Mat& image)
{
	std::mutex faceDetectorMutex;
	cv::CascadeClassifier faceDetector;

	std::vector<cv::Rect> faces;
	if (faceDetector.empty())
	{
		return faces;
	}

	cv::Mat gray;
	cvtColor(image, gray, cv::COLOR_BGR2GRAY);

	std::lock_guard<std::mutex> lock(faceDetectorMutex);
	faceDetector.detectMultiScale(gray, faces, 1.15, 3, 0 | 
		//CV_HAAR_DO_CANNY_PRUNING,
		//CV_HAAR_DO_ROUGH_SEARCH,
		//CV_HAAR_FEATURE_MAX,
		CV_HAAR_FIND_BIGGEST_OBJECT,
		//CV_HAAR_SCALE_IMAGE,
		cv::Size(70,70));

	return faces;
}

void findFace(PersonDetect& pd,Mat frame)
{
	std::vector<cv::Rect> faces;
	faces = getFaces(frame);
	joinOverlapped(faces);
	if (faces.empty())
	{
		return;
	}

	cv::Rect faceRect;
	faceRect = *std::max_element(faces.begin(), faces.end(),
		[] (cv::Rect lFace, cv::Rect rFace)
		{
			return lFace.area() < rFace.area();
		});

	double yDiff = faceRect.height * 0.1;
	faceRect.y -= yDiff;
	faceRect.height += yDiff * 2;
	auto faceImg = getSafeRoi(frame, faceRect);
	//pd.setHead(faceImg, true);
	pd.setHead(faceRect, true);
}

bbox_t assingToBestDetect(bbox_t headBox,
	const std::vector<PersonDetect>& detects,
	std::vector<std::pair<bbox_t, double>>& assignments)
{
	double minDist = -1.0;
	int bestDetectID = -1;
	cv::Point centerHead(headBox.x + headBox.w / 2, headBox.y + headBox.h / 2);
	for (int i = 0; i < detects.size(); i++)
	{
		//skip body if not close to it
		auto body = detects[i].getBody().frameRect;
		cv::Point possibleHeadLocation(body.x + body.width / 2, body.y + body.height / 8);
		if (!isPointInSegmentBox(Segment{ cv::Point(body.x, body.y),
					cv::Point(body.x + body.width, body.y + body.height) },
				centerHead))
		{
			continue;
		}
		auto distance = distPointToPoint(centerHead, possibleHeadLocation);
		//skip body that has better assignment
		if (assignments[i].second != -1.0 &&
			distance >= assignments[i].second)
		{
			continue;
		}
		if (bestDetectID == -1 ||
			distance < minDist)
		{
			bestDetectID = i;
			minDist = distance;
		}
	}

	if (bestDetectID != -1)
	{
		bbox_t oldBox = assignments[bestDetectID].first;
		assignments[bestDetectID] = std::pair<bbox_t, double>(headBox, minDist);
		if (boxToRect(oldBox) != boxToRect(bbox_t()))
		{
			return assingToBestDetect(headBox, detects, assignments);
		}
		else
		{
			return oldBox;
		}
	}
	return headBox;
}

std::vector<PersonDetect> getDetects(const std::vector<bbox_t>& boxes, const cv::Mat& frame)
{
	std::vector<bbox_t> bodies;
	std::vector<bbox_t> headsAndFaces;
	for (auto box : boxes) //3 - body, 2 - head, 1 - face
	{
		if (box.obj_id == 3)
		{
			bodies.push_back(box);
		}
		else if (box.obj_id == 2 &&
			std::find_if(boxes.begin(), boxes.end(),
				[box] (bbox_t other)
				{
					//change box for box + margin
					cv::Point margin(box.w * 0.1, box.h * 0.1);
					auto expandedRect = (boxToRect(box) - margin) + cv::Size(margin * 2);
					return other.obj_id == 1 && (expandedRect & boxToRect(other)) == boxToRect(other);
				}) != boxes.end())
		{
			continue;
		}
		else if (box.obj_id == 2 || box.obj_id == 1)
		{
			headsAndFaces.push_back(box);
		}
	}
	//-----CLEAR SIMILAR DETECTS-----
	joinOverlapped(bodies);
	joinOverlapped(headsAndFaces);
	//====================================
	//-----CREATE DETECTS FROM BODIES-----
	std::vector<PersonDetect> detects;
	for (auto box : bodies) //create objects for every detected body
	{
		PersonDetect pd("cfg.office", "cfg.camId");
		auto bodyRect = boxToRect(box);
		Safe_dimensions(bodyRect, bodyRect, frame,
			bodyRect.width * 0.1, 0,
			bodyRect.width * 0.1, 0);
		pd.setBody(bodyRect);
		detects.push_back(pd);
	}
	//====================================
	std::vector<std::pair<bbox_t, double>> assignments(detects.size(),
		std::pair<bbox_t, double>(bbox_t(), -1.0));
	//-----ADD HEADS TO EXISTING DETECTS OR CREATE NEW-----
	std::vector<bbox_t> unsuitableBoxes;
	for (auto box : headsAndFaces)
	{
		bbox_t unsuitableBox = assingToBestDetect(box, detects, assignments);
		if (boxToRect(unsuitableBox) != boxToRect(bbox_t()))
		{
			unsuitableBoxes.push_back(unsuitableBox);
		}
	}
	for (int i = 0; i < assignments.size(); i++)
	{
		//-----APPLY EVERY ASSIGNMENT-----
		if (boxToRect(assignments[i].first) != boxToRect(bbox_t()))
		{
			auto headRect = boxToRect(assignments[i].first);
			Safe_dimensions(headRect, headRect, frame,
				headRect.width * 0.05, headRect.height * 0.1,
				headRect.width * 0.05, headRect.height * 0.1);
			bool isFace = assignments[i].first.obj_id == 1;
			detects[i].setHead(headRect, isFace);
		}
	}
	for (auto box : unsuitableBoxes)
	{
		//-----CREATE NEW DETECTS FOR ALL UNASSIGNED HEADS-----
		PersonDetect pd("cfg.office", "cfg.camId");
		auto headRect = boxToRect(box);
		Safe_dimensions(headRect, headRect, frame,
			headRect.width * 0.05, headRect.height * 0.1,
			headRect.width * 0.05, headRect.height * 0.1);
		bool isFace = box.obj_id == 1;
		pd.setHead(headRect, isFace);
		detects.push_back(pd);
	}
	return detects;
}
*/
void runCam(Camera camera, std::shared_ptr<ThreadSafeDetector> detector)
{
	//printf("PID of this process: %d\n", getpid());
	std::cout << "Thread No: " << pthread_self() << std::endl;
	std::cout<<camera.getPath()<<std::endl;

	bool isJustStartWriteVideo=false;

	while(true)
	{
		camera.getFrame();
		/*std::vector<bbox_t>boxes;
		std::vector<cv::Rect>v_rect;
		camera.getFrame();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if(!camera.frame.empty())
		boxes = detector->detect(camera.frame);
		
		for(auto box:boxes)				
			v_rect.push_back(cv::Rect(box.x,box.y,box.w,box.h));
		*/
		//camera.setDrawRectangles(v_rect);

		//if((faceDetector.getFaces(camera[numCamera].frame)).size()!=0)std::cout<<",";	
		if(camera.move_detect(70) )
		{
			if(!isJustStartWriteVideo)
			{
				camera.setTimer();
			}
			isJustStartWriteVideo=true;
			camera.startWrite();
			camera.setDuration(maxDuration);	
		}
		if(!camera.getDuration() || camera.isTimeUp())
		{			
			isJustStartWriteVideo=false;
			camera.stopWrite();
		}
		else camera.duration--;
	}
}

int main() {
	//FaceDetector faceDetector;
	//faceDetector.init_face_detect();
	
	const std::string DARKNET_CONFIG_PATH = "data/yolov3-tiny_obj.cfg";
	const std::string DARKNET_WEIGHTS_PATH = "data/yolov3-tiny_obj_19200.weights";
	std::shared_ptr<ThreadSafeDetector> detector =
		std::make_shared<ThreadSafeDetector>(DARKNET_CONFIG_PATH, DARKNET_WEIGHTS_PATH, 0);

	auto camListCfg = getParams("config.cfg", "CamList", { "cams" });
	std::vector<std::string> camList;
	try
	{
		camList = split(camListCfg.at("cams"), ',',true);
	}
	catch (const std::out_of_range& ex)
	{
		std::cout << "Cannot find CamList in config file: " << ex.what() << std::endl;
	}

	int numCamera=0;
	vector<String> urlOfCamera;
	Camera *camera= new Camera [camList.size()];


	for (auto camCfgName : camList)
	{
		Camera::Config cfg;
		cfg.parseParams(camCfgName);		
		urlOfCamera.push_back(cfg.url);

		camera[numCamera].setNameVideoCamName(cfg.camId);
		camera[numCamera].setPath(cfg.url);	
		camera[numCamera].setNameVideoCab(cfg.office);
		camera[numCamera].setConfig(cfg);
	    camera[numCamera].initCamera();	   

		numCamera++;
	}
	
	numCamera=0;	

	std::cout << "Start program" << std::endl;

	//printf("PID of main process: %d\n", getpid());
	std::cout << "Thread main No: " << pthread_self() << std::endl;

	std::vector<std::shared_ptr<std::thread>>thr;
	for (auto unit : camList)
	{
		thr.push_back(std::make_shared<std::thread>(runCam,std::ref(camera[numCamera]),detector));
		//thr[numCamera]->join();
		std::cout << "Run thread" << std::endl;
		numCamera++;
	}

	numCamera=0;
	
	while(1){};
	delete [] camera;
	return 0;
}


/*
//Camera camera1("rtsp://admin:startup123@192.168.1.220:554/Streaming/Channels/102/");
Camera camera2("rtsp://admin:startup123@192.168.1.222:554/Streaming/Channels/102/");
Camera camera1("rtsp://admin:admin@192.168.90.169:554/MPEG-4/ch12/main/av_stream");

//int maxDuration=80;	


int main() {

	bool isJustStartWriteVideo=false;
	std::string faceCascadeName = "haarcascade_frontalface_alt2.xml";
	std::shared_ptr<ThreadSafeDetector> detector =
		std::make_shared<ThreadSafeDetector>(DARKNET_CONFIG_PATH, DARKNET_WEIGHTS_PATH, 0);
	
	camera1.setNameVideoCamName("cam1");
	camera2.setNameVideoCamName("cam2");
	
	CTracker tracker(true, kalmanStep, accelNoise, distThresh, skipFrames, maxTraceLength);
	
	std::cout << "Start program" << std::endl;
	
	while (true) {
		
		camera1.getFrame();	
		if(camera1.frame.empty())
		{
			auto boxes = detector->detect(camera1.frame);
			std::vector<PersonDetect> detects = getDetects(boxes, camera1.frame);
	
			Mat grayFrame;
			cv::cvtColor(camera1.frame, grayFrame, cv::COLOR_BGR2GRAY);
			tracker.Update(0,detects,CTracker::RectsDist,grayFrame);
	
			if(camera1.move_detect(70) )
			{
				if(!isJustStartWriteVideo)
				{
					camera1.setTimer();
				}
				isJustStartWriteVideo=true;
				camera1.startWrite();
				camera1.setDuration(maxDuration);	
			}
			if(!camera1.getDuration() || camera1.isTimeUp())
			{			
				isJustStartWriteVideo=false;
				camera1.stopWrite();
			}
			else camera1.duration--;
		}
		//camera2.getFrame();		
		//if(camera2.move_detect(70) )
		//	{
		//		camera2.canWrite=true;
		//		camera2.duration=maxDuration;				
		//	}
		//if(!camera2.duration)
		//{			
		//	camera2.canWrite=false;
		//}
		//else camera2.duration--;
       
	}
	return 0;
}*/