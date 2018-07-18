#include "ThreadSafeDetector.h"


std::vector<bbox_t> ThreadSafeDetector::detect(const cv::Mat& frame, float thresh)
{
	try
	{
		std::lock_guard<std::mutex> lock(m);
		return Detector::detect(frame, thresh);
	}
	catch (const std::exception & ex)
	{
		std::cout << ex.what() << std::endl;
		return std::vector<bbox_t>();
	}
}