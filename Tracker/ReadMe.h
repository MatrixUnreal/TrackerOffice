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

	#define Mat_t CV_32FC

	typedef std::vector<CRegion> regions_t;

	typedef std::vector<std::unique_ptr<CTrack>> tracks_t;

	Критерии прекращения итерационного алгоритма поиска (после указанного максималь-
ного числа итераций criteria.maxCount, или когда окно поиска станет меньше, чем criteria.epsilon).
	cv::TermCriteria termcrit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);