#pragma once
#include "defines.h"

const cv::Size defaultSize(800, 600);
//
static const track_t kalmanStep = 0.2;//0.1;    //|
										  //_______________________________________________|
										  //											     |
static const track_t accelNoise = 0.1;	   //|
										   //_______________________________________________|
										   //											|
static const track_t distThresh = 300;	   //|
										   //_______________________________________________|
										   //											|
static const size_t skipFrames = 10;//30;	   //|
										   //_______________________________________________|
										   //
static const size_t	maxTraceLength = 200;//500;	  //|
											  //_______________________________________________|
											  //
static const float minObjectWidth = 0.1f;   //|
											 //_______________________________________________|
											 //
static const float minObjectHeight = 0.1f;   //|
											  //_______________________________________________| 
											  //
static const double subtractThresh = 20;	   //|
											   //_______________________________________________|
