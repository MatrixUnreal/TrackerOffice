#include "Video.h"

namespace fs = std::experimental::filesystem;

void Video::createMainVideoFolder()
{
	const fs::path workdir = fs::current_path();
	if (!fs::exists(workdir / "Video"))
	{
		fs::create_directory(workdir / "Video");
		std::cout << "Create  " << (workdir / "Video") << std::endl;
	}
}

void Video::createDateVideoFolder()
{
	createMainVideoFolder();

	time_t     now = time(0);
    struct tm  tstruct;
    char       month[20];
    char       day[10];
    tstruct = *localtime(&now);
    strftime(month, sizeof(month), "%Y-%m", &tstruct);
    strftime(day, sizeof(day), "%d", &tstruct);
	const fs::path videoDir = fs::current_path()/ "Video";
	if (!fs::exists(videoDir / month))
	{
		fs::create_directory(videoDir / month );
		std::cout << "Create  " << (videoDir / month ) << std::endl;
	}
	if (!fs::exists(videoDir / month / day))
	{
		fs::create_directory(videoDir / month / day );
		std::cout << "Create  " << (videoDir / month / day ) << std::endl;
	}
}