#define _USE_MATH_DEFINES

#include <cmath>

#include "mymath.h"


Line toLine(Segment s)
{
	double a = s.end.y - s.start.y;
	double b = s.start.x - s.end.x;

	return Line{ a, b, -a * s.start.x - b * s.start.y };
}

cv::Point closestPoint(Line l, cv::Point p)
{
	double k = (l.a * p.x + l.b * p.y + l.c) / (l.a * l.a + l.b * l.b);
	return cv::Point(p.x - l.a * k, p.y - l.b * k);
}

double distPointToPoint(cv::Point p1, cv::Point p2)
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

double distPointToSegment(Segment s, cv::Point p)
{
	cv::Point t = closestPoint(toLine(s), p);

	if (isPointInSegmentBox(s, t))
		return distPointToPoint(p, t);
	else
		return std::min(distPointToPoint(p, s.start), distPointToPoint(p, s.end));
}

bool areSegmentsCrossed(Segment a, Segment b)
{
	return isPointOnSegment(a, b.start)
		|| isPointOnSegment(a, b.end)
		|| isPointOnSegment(b, a.start)
		|| isPointOnSegment(b, a.end)
		|| ((isPointRightOfLine(a, b.start) ^ isPointRightOfLine(a, b.end))
			&& (isPointRightOfLine(b, a.start) ^ isPointRightOfLine(b, a.end)));
}

bool isPointInSegmentBox(Segment a, cv::Point b)
{
	auto minmaxX = std::minmax(a.start.x, a.end.x);
	auto minmaxY = std::minmax(a.start.y, a.end.y);
	return (b.x >= minmaxX.first && b.x <= minmaxX.second)
		&& (b.y >= minmaxY.first && b.y <= minmaxY.second);
}

bool isPointOnSegment(Segment a, cv::Point b)
{
	return isPointOnLine(a, b) && isPointInSegmentBox(a, b);
}

bool isPointOnLine(Segment a, cv::Point b)
{
	Segment aTmp = Segment{ cv::Point(0, 0), cv::Point(a.end.x - a.start.x, a.end.y - a.start.y) };
	cv::Point bTmp = cv::Point(b.x - a.start.x, b.y - a.start.y);
	double r = crossProduct(aTmp.end, bTmp);
	return abs(r) < M_E;
}

bool isPointRightOfLine(Segment a, cv::Point b)
{
	Segment aTmp = Segment{ cv::Point(0, 0), cv::Point(a.end.x - a.start.x, a.end.y - a.start.y) };
	cv::Point bTmp = cv::Point(b.x - a.start.x, b.y - a.start.y);
	return crossProduct(aTmp.end, bTmp) < 0;
}

bool lineSegmentTouchesOrCrossesLine(Segment a, Segment b)
{
	return isPointOnLine(a, b.start)
		|| isPointOnLine(a, b.end)
		|| (isPointRightOfLine(a, b.start) ^ isPointRightOfLine(a, b.end));
}

double crossProduct(cv::Point a, cv::Point b)
{
	return a.x * b.y - b.x * a.y;
}