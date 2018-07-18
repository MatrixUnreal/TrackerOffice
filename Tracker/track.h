#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <array>

#include "opencv2/opencv.hpp"

#include "defines.h"
#include "Kalman.h"
#include "Types.h"
#include "PersonDetect.h"

// --------------------------------------------------------------------------
class CTrack
{
public:
	CTrack(int frameId,
		PersonDetect detect,
		track_t dt,
		track_t Accel_noise_mag,
		size_t trackID)
		:
		track_id(trackID),
		skipped_frames(0),
		lastRegion(detect.getRect()),
		pointsCount(0),
		prediction(detect.getPoint()),
		KF(Point_t(detect.getPoint()), dt, Accel_noise_mag)
	{
		srand(time(0));
		color = cv::Scalar(rand() % 256, rand() % 256, rand() % 256);
		detects.push_back(std::pair<int, PersonDetect>(frameId, detect));
	}

	track_t CalcDist(const Point_t& p)
	{
		Point_t diff = prediction - p;
		return sqrtf(diff.x * diff.x + diff.y * diff.y);
	}

	track_t CalcDist(const cv::Rect& r)
	{
		std::array<track_t, 4> diff;
        diff[0] = prediction.x - lastRegion.m_rect.width / 2 - r.x;
        diff[1] = prediction.y - lastRegion.m_rect.height / 2 - r.y;
        diff[2] = static_cast<track_t>(lastRegion.m_rect.width - r.width);
        diff[3] = static_cast<track_t>(lastRegion.m_rect.height - r.height);

		track_t dist = 0;
		for (size_t i = 0; i < diff.size(); ++i)
		{
			dist += diff[i] * diff[i];
		}
		return sqrtf(dist);
	}

    void Update(int frameId, PersonDetect detect, size_t max_trace_length)
	{
		KF.GetPrediction();

		bool dataCorrect = detect.getHead().frameRect.area() > 0 || detect.getBody().frameRect.area() > 0;
		auto p = Point_t();
		auto region = CRegion();
		if (dataCorrect)
		{
			lastFrameTime = detect.getDetectionTime();
			region = detect.getRect();
			p = Point_t(detect.getPoint());
			detects.push_back(std::pair<int, PersonDetect>(frameId, detect));
		}

        if (pointsCount)
        {
            if (dataCorrect)
            {
                prediction = KF.Update((p + averagePoint) / 2, dataCorrect);
                //prediction = (prediction + averagePoint) / 2;
            }
            else
            {
                prediction = KF.Update(averagePoint, dataCorrect);
            }
        }
        else
        {
            prediction = KF.Update(p, dataCorrect);
        }

		if (dataCorrect)
		{
			lastRegion = region;
		}

		if (trace.size() > max_trace_length)
		{
			trace.erase(trace.begin(), trace.end() - max_trace_length);
		}

		trace.push_back(prediction);
	}

	cv::Scalar color;
	time_t lastFrameTime = time(0);
	//first - id of frame where detect was detected, second - detect itself
	std::vector<std::pair<int, PersonDetect>> detects;
	std::vector<Point_t> trace;
	size_t workerId = 0;
	size_t track_id;
	size_t skipped_frames;
    CRegion lastRegion;
    int pointsCount;
    Point_t averagePoint;

	cv::Rect GetLastRect()
	{
		return cv::Rect(
            static_cast<int>(prediction.x - lastRegion.m_rect.width / 2),
            static_cast<int>(prediction.y - lastRegion.m_rect.height / 2),
            lastRegion.m_rect.width,
            lastRegion.m_rect.height);
	}

	void setWorkerId()
	{
		std::map<size_t, int> counts;
		size_t bestWorkerId = 0;
		int maxCount = 0;
		for (auto& detect : detects)
		{
			if (!detect.second.hasName())
				continue;
			
			size_t tmpWorkerId = stoi(detect.second.getName());
			auto inserted = counts.insert(std::pair<size_t, int>(tmpWorkerId, 1));
			if (!inserted.second)
			{
				counts[tmpWorkerId]++;
				if (counts[tmpWorkerId] > maxCount)
				{
					maxCount = counts[tmpWorkerId];
					bestWorkerId = tmpWorkerId;
				}
			}
		}
		if (counts[bestWorkerId] > 1)
			workerId = bestWorkerId;
	}

private:
	Point_t prediction;
	TKalmanFilter KF;
};

typedef std::vector<std::unique_ptr<CTrack>> tracks_t;


class Track
{
public:
	Track()
		: color(rand() % 256, rand() % 256, rand() % 256)
	{}
	Track(const CTrack* const track)
		:
		id(track->track_id), detects(track->detects), color(track->color), workerId(track->workerId)
	{}

	void setWorkerId();

	std::vector<PersonDetect> getSomeDetects() const;

//private:
	size_t id;
	size_t workerId = 0;
	std::vector<std::pair<int, PersonDetect>> detects;
	std::vector<detectedEvent> events;
	cv::Scalar color;
};