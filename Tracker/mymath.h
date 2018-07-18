#pragma once

#include <opencv2/core.hpp>


struct Segment
{
	cv::Point start;
	cv::Point end;
};

struct Line
{
	double a;
	double b;
	double c;
};


Line toLine(Segment s);
cv::Point closestPoint(Line l, cv::Point p);
double distPointToPoint(cv::Point p1, cv::Point p2);
double distPointToSegment(Segment s, cv::Point p);
bool areSegmentsCrossed(Segment a, Segment b);
bool isPointInSegmentBox(Segment a, cv::Point b);
bool isPointOnSegment(Segment a, cv::Point b);

bool isPointOnLine(Segment a, cv::Point b);
bool isPointRightOfLine(Segment a, cv::Point b);
bool lineSegmentTouchesOrCrossesLine(Segment a, Segment b);

double crossProduct(cv::Point a, cv::Point b);
