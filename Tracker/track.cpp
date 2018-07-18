#include "track.h"

#include <vector>
#include <algorithm>
#include <iterator>


void Track::setWorkerId()
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

std::vector<PersonDetect> Track::getSomeDetects() const
{
	std::vector<PersonDetect> tmp;
	std::transform(detects.begin(), detects.end(), std::back_inserter(tmp),
		[] (const std::pair<int, PersonDetect>& p) { return p.second; });
	std::sort(tmp.begin(), tmp.end(),
		[] (const PersonDetect& left, const PersonDetect& right)
		{
			if (left.getCoef() != -1 && right.getCoef() == -1)
				return true;
			return left.getCoef() < right.getCoef();
		});
	//tmp: { good }, { bad }, { -1 }

	std::vector<PersonDetect> ret;
	auto it = tmp.begin();
	//write first three (best detects) that have coefficient != -1
	for (int i = 0; i < 3 && it != tmp.end() && it->getCoef() != -1; i++, it++)
	{
		ret.push_back(*it);
	}
	//write every 3rd while coefficient != -1
	//then write every 4th
	for (int n = 3, i = 1; it != tmp.end(); i++, it++)
	{
		if (it->getCoef() == -1)
			n = 4;
		if (i % n == 0)
			ret.push_back(*it);
	}
	return ret;
}