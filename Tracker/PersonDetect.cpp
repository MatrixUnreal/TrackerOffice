#include <system_error>
#include <experimental/filesystem>

#include "PersonDetect.h"
#include "Util.hpp"

namespace fs = std::experimental::filesystem;


const cv::Size PersonDetect::maxBodySize = cv::Size(300, 700);
const cv::Size PersonDetect::maxHeadSize = cv::Size(200, 200);


void PersonDetect::setBody(cv::Rect bodyRect)
{
	body.frameRect = bodyRect;
}

void PersonDetect::setHead(cv::Rect headRect, bool isFace)
{
	head.frameRect = headRect;
	_hasFace = isFace;
}

void PersonDetect::setName(const std::string& _name, double _coef)
{
	name = _name;
	coef = _coef;
}

std::vector<std::string> PersonDetect::save(const cv::Mat& parentImage)
{
	const int BUFF_SIZE = 50;
	char timeStr[BUFF_SIZE];
	strftime(timeStr, BUFF_SIZE, "%Y-%m-%d_%H-%M-%S", localtime(&detectionTime));
	std::string extension = ".jpg";

	fs::path dirBody = "Bodies";
	fs::path dirRecognized = "Image_Recognized";
	fs::path dirNotRecognized = "Image_Not_Recognized";
	char buff[BUFF_SIZE];
	strftime(buff, BUFF_SIZE, "%Y-%m", localtime(&detectionTime));
	dirBody /= std::string(buff);
	dirRecognized /= std::string(buff);
	dirNotRecognized /= std::string(buff);
	std::fill(buff, buff + BUFF_SIZE, 0);
	strftime(buff, BUFF_SIZE, "%d", localtime(&detectionTime));
	dirBody /= std::string(buff);
	dirRecognized /= std::string(buff);
	dirNotRecognized /= std::string(buff);
	std::fill(buff, buff + BUFF_SIZE, 0);

	std::error_code ec;
	fs::create_directories(dirBody, ec);
	fs::create_directories(dirRecognized, ec);
	fs::create_directories(dirNotRecognized, ec);

	std::vector<std::string> filenames;
	if (hasBody())
	{
		std::string imageInfo = std::string(timeStr) + "=" + camID + "=" + office + "=" + "b_";
		size_t index = 0;
		fs::path filename = imageInfo + std::to_string(index) + extension;
		while (fs::exists(dirBody / filename, ec))
		{
			++index;
			filename = imageInfo + std::to_string(index) + extension;
		}
		body.filename = (dirBody / filename).string();
		auto bodyImage = getSafeRoi(parentImage, body.frameRect);
		cv::imwrite(body.filename, bodyImage);
		filenames.push_back(body.filename);
	}

	if (hasHead())
	{
		std::string imageInfo = std::string(timeStr) + "=" + camID + "=" + office + "=";
		imageInfo += (hasFace() ? "f_" : "h_");
		if (hasName())
		{
			imageInfo += name;
		}
		size_t index = 0;
		fs::path& dir = (hasName() ? dirRecognized : dirNotRecognized);
		fs::path filename = imageInfo + std::to_string(index) + extension;
		while (fs::exists(dir / filename, ec))
		{
			++index;
			filename = imageInfo + std::to_string(index) + extension;
		}
		head.filename = (dir / filename).string();
		auto headImage = getSafeRoi(parentImage, head.frameRect);
		cv::imwrite(head.filename, headImage);
		filenames.push_back(head.filename);
	}

	return filenames;
}

cv::Rect PersonDetect::getRect() const
{
	if (getHead().frameRect.area() > 0)
	{
		return getHead().frameRect;
	}
	else
	{
		auto body = getBody().frameRect;
		return cv::Rect(body.x + body.width / 4, body.y, body.width / 2, body.height / 6);
	}
}

Point_t PersonDetect::getPoint() const
{
	auto rect = getRect();
	return Point_t((rect.br() + rect.tl()) * 0.5, detectionTime);
}