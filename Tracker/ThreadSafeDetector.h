#pragma once
#include <vector>
#include <mutex>

#include <opencv2/opencv.hpp>
#include "yolo_v2_class.hpp"


class ThreadSafeDetector : public Detector
{
public:
	ThreadSafeDetector(const std::string& cfgPath, const std::string& weightsPath, int gpuId = 0)
		: Detector(cfgPath, weightsPath, gpuId)
	{}

	std::vector<bbox_t> detect(const cv::Mat& frame, float thresh = 0.5);
	
private:
	ThreadSafeDetector(const ThreadSafeDetector& other) = delete;
	ThreadSafeDetector(ThreadSafeDetector&& other) = delete;

	std::mutex m;
};