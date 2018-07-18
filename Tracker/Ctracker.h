#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <map>

#include "defines.h"
#include "track.h"
#include "HungarianAlg.h"
#include "LocalTracker.h"
#include "PersonDetect.h"
#include "Util.hpp"


// --------------------------------------------------------------------------
class CTracker
{
public:
	CTracker(bool useLocalTracking,
             track_t dt_,
             track_t Accel_noise_mag_,
             track_t dist_thres_ = 50,
             size_t maximum_allowed_skipped_frames_ = 10,
             size_t max_trace_length_ = 100);
	~CTracker(void);

	enum DistType
	{
		CentersDist = 0,
		RectsDist = 1
	};

	tracks_t tracks;
	std::vector<Track> deadTracks;
	void Update(int frameId,
		const std::vector<PersonDetect>& detects,
		DistType distType,
		cv::Mat gray_frame);

private:
	// Use local tracking for regions between two frames
	bool m_useLocalTracking;

	// Шаг времени опроса фильтра
	track_t dt;

	track_t Accel_noise_mag;

	// Порог расстояния. Если точки находятся дуг от друга на расстоянии,
	// превышающем этот порог, то эта пара не рассматривается в задаче о назначениях.
	track_t dist_thres;
	// Максимальное количество кадров которое трек сохраняется не получая данных о измерений.
	size_t maximum_allowed_skipped_frames;
	// Максимальная длина следа
	size_t max_trace_length;

	size_t NextTrackID;

    LocalTracker localTracker;
};
