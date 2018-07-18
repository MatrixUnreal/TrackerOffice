#pragma once
#include <string>
#include <vector>
#include <map>

#include <opencv2/opencv.hpp>


const cv::Size DEFAULT_SIZE = cv::Size(1800, 1800);//(800, 600);
const std::string VIDEO_DIR = "Video";
const std::string VIDEO_EXTENSION = ".mp4";
const std::string DARKNET_CONFIG_PATH =  "Tracker/head/yolo-obj.cfg";
const std::string DARKNET_WEIGHTS_PATH = "Tracker/head/yolo-obj_14000.weights";

//*****DATABASE*****
const std::string DB_TABLE_WORKER = "worker";
const std::string DB_TABLE_PERMISSION = "perms";
const std::string DB_TABLE_IMAGE = "img";
const std::string DB_TABLE_VIDEO = "videoclips";
const std::string DB_TABLE_EVENT = "entry";
//==================

const std::map<std::string, std::vector<int>> codecs = {
	std::pair<std::string, std::vector<int>>(".mp4", {
		cv::VideoWriter::fourcc('A', 'V', 'C', '1'),
		cv::VideoWriter::fourcc('H', '2', '6', '4'),
		cv::VideoWriter::fourcc('X', '2', '6', '4'),
		cv::VideoWriter::fourcc('M', 'P', '4', 'V')
	})
};