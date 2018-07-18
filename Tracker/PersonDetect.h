#pragma once
#include <string>
#include <vector>
#include <ctime>

#include <opencv2/opencv.hpp>

#include "defines.h"


class PersonDetect
{
public:
	struct BodyPart
	{
		cv::Rect frameRect = cv::Rect(0, 0, 0, 0);
		std::string filename; //empty string by default; path to file after save() have been called
	};

	PersonDetect() {}
	PersonDetect(const std::string& _office,
		const std::string& _camID)
	: office(_office), camID(_camID) 
	{}

	void setId(size_t n) { id = n; }
	/*
	@param _body - image of body
	*/
	void setBody(cv::Rect bodyRect);
	/*
	@param _head - image of head
	*/
	void setHead(cv::Rect headRect, bool isFace = false);
	void setName(const std::string& _name, double _coef);
	std::vector<std::string> save(const cv::Mat& parentImage); //TODO: rework or move

	/*Need this for tracker*/
	cv::Rect getRect() const;
	/*Need this for tracker
	@return - middle point of head or
	point that lies in the middle horizontally and almost at the top of body*/
	Point_t getPoint() const;
	size_t getId() const { return id; }
	std::string getName() const { return name; }
	double getCoef() const { return coef; }
	bool isEmpty() const { return !hasBody() && !hasHead(); }
	bool hasName() const { return name != "UNRECOGNIZED" && !name.empty(); }
	bool hasFace() const { return _hasFace; }
	bool hasBody() const { return getBody().frameRect.area() > 0; }
	bool hasHead() const { return getHead().frameRect.area() > 0; }
	time_t getDetectionTime() const { return detectionTime; }
	const BodyPart& getBody() const { return body; }
	const BodyPart& getHead() const { return head; }

private:
	static const cv::Size maxBodySize;
	static const cv::Size maxHeadSize;

	size_t id = 0;
	std::string office;
	std::string camID;
	std::string name;
	BodyPart body;
	BodyPart head;
	double coef = -1.0;
	bool _hasFace = false;
	time_t detectionTime = time(0);
};