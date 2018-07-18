#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

typedef float track_t;
//typedef cv::Point_<track_t> Point_t;
struct Point_t : public cv::Point_<track_t>
{
	time_t t;

	Point_t(time_t _t = time(0))
        : cv::Point_<track_t>(), t(_t) {}
	Point_t(const cv::Point_<track_t> & p, time_t _t = time(0))
        : cv::Point_<track_t>(p), t(_t) {}
	Point_t(int x, int y, time_t _t = time(0))
        : cv::Point_<track_t>(x, y), t(_t) {}
};
#define Mat_t CV_32FC

class CRegion
{
public:
    CRegion()
    {
    }

    CRegion(const cv::Rect& rect)
        : m_rect(rect)
    {

    }

    cv::Rect m_rect;
    std::vector<cv::Point2f> m_points;
};

typedef std::vector<CRegion> regions_t;
