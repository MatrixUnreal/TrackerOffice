#include "MyCamera.h"

using namespace std;
using namespace cv;

const char * const CAM_CONFIG_NAME = "config.cfg";


bool Camera::move_detect(int sensitivity)
{
	if(this->frame.empty() ||this->lastframe.empty())return false;

	cv::Mat imgDifference, gray;
	absdiff(this->frame, this->lastframe, imgDifference);
	IplImage* _image = new IplImage(imgDifference);

	IplImage* bin;
	cv::waitKey(1);
	assert(_image != 0);
	bin = cvCreateImage(cvGetSize(_image), IPL_DEPTH_8U, 1);
	cvConvertImage(_image, bin, CV_BGR2GRAY);
	cvCanny(bin, bin, 50, 200);
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours = 0;
	int contours_count = cvFindContours(bin, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	cvtColor(this->frame, gray, cv::COLOR_BGR2GRAY);

	for (CvSeq* current = contours; current != NULL; current = current->h_next)
	{
		CvPoint2D32f center;
		float radius = 0;

		cvMinEnclosingCircle(current, &center, &radius);
		if (radius > sensitivity)
		{
			std::cout << "_" << std::endl;
			cvReleaseMemStorage(&storage);
			cvReleaseImage(&bin);
			delete(_image);
			return true;
		}
	}

	cvReleaseMemStorage(&storage);
	cvReleaseImage(&bin);
	delete(_image);
	return false;
}

Camera::Camera(String path)
{
	this->path=path;
	this->vcap.open(path);
	if (!this->vcap.isOpened()) {
	cout << "Error opening video stream or file" << endl;
	}
	frame_width = this->vcap.get(CV_CAP_PROP_FRAME_WIDTH);
    frame_height = this->vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    if(this->config.rotate!=0){
    	frame_height = this->vcap.get(CV_CAP_PROP_FRAME_WIDTH);
    	frame_width = this->vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    };
	frame_rate = this->vcap.get(CAP_PROP_FPS);
	nameVideoFirstPart=currentDateTime();	
}

void Camera::initCamera()
{
	this->vcap.open(path);
	if (!this->vcap.isOpened()) {
	cout << "Error opening video stream or file" << endl;
	cout << "path= "<<path<<endl;
	}
	frame_width = this->vcap.get(CV_CAP_PROP_FRAME_WIDTH);
    frame_height = this->vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    if(this->config.rotate!=0){
    	frame_height = this->vcap.get(CV_CAP_PROP_FRAME_WIDTH);
    	frame_width = this->vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    };
	frame_rate = this->vcap.get(CAP_PROP_FPS);
	nameVideoFirstPart=currentDateTime();
}

bool Camera::getFrame()
{
	for(int i=0;i<maxBadFrame;i++)
	{
		this->lastframe = this->frame.clone();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (!this->vcap.read(this->frame)) // if not success, break loop
    	{
    	    cout<<"\n Cannot read the frame from "<<this->nameVideoCamName <<std::endl;
    	}
    	else
    	{
    		break;
    	}

    	if(i>=4)
    	{
    		this->vcap.release();
    		cout<<"Reconect to camera "<<this->nameVideoCamName <<std::endl;
    		initCamera();
    	}
	}

	if(this->config.rotate==1)
	{
		cv::rotate(this->frame, this->frame, ROTATE_90_CLOCKWISE);
	}
	else if(this->config.rotate==2)
	{
		cv::rotate(this->frame, this->frame, ROTATE_90_COUNTERCLOCKWISE);
	}

	cv::waitKey(10);
	time_t     now = time(0);
    struct tm  tstruct;
    char       month[20];
    char       day[10];
    tstruct = *localtime(&now);
    strftime(month, sizeof(month), "%Y-%m", &tstruct);
    strftime(day, sizeof(day), "%d", &tstruct);

	if(canWrite) 
		{	
			if(!this->video.isOpened())
			{
				Video::createDateVideoFolder();
				this->pathWriter=this->nameVideoFolder+month+"/"+day+"/"+this->nameVideoFirstPart+"="+this->nameVideoCab+"="+this->nameVideoCamName+this->nameVideoExtention;
				std::cout << "Start record from " <<this->nameVideoCamName << std::endl;
				this->video.open(pathWriter, CV_FOURCC('M', 'P', '4', 'V'), frame_rate? frame_rate:15, Size(frame_width, frame_height), true);
				//this->video.open(pathWriter, CV_FOURCC('H', '2', '6', '4'), frame_rate? frame_rate:15, Size(frame_width, frame_height), false);
				//this->video.open(pathWriter, CV_FOURCC('M','J','P','G'), frame_rate? frame_rate:15, Size(frame_width, frame_height), true);
			}
			if(this->frame.empty())
			{
				std::cout << "frame is empty from " <<this->nameVideoCamName <<std::endl;
			}
			else if(this->frame.cols!=frame_width || this->frame.rows!=frame_height ||this->frame.empty())
			{
				std::cout << "frame is wrong from " <<this->nameVideoCamName <<" "<<this->frame.cols<<"x"<<this->frame.rows<<std::endl;
			}			
			else
			{
				 srand ( time(NULL) );
				for(auto rect:rects)
				cv::rectangle(this->frame, rect, cv::Scalar((rand() % 255) ,(rand() % 255),(rand() % 255)), 1, LINE_8, 0 );
				this->video.write(this->frame);
			}
		}
	else 
		{
			if(this->video.isOpened()){
				std::cout << "Stop record from " <<this->nameVideoCamName << std::endl;
				this->video.release();
				std::cout << currentDateTime() << std::endl;
				cv::String newName=this->nameVideoFolder+month+"/"+day+"/"+this->nameVideoFirstPart+"="+currentDateTime()+"="+this->nameVideoCab+"="+this->nameVideoCamName+this->nameVideoExtention;
				
				if ( rename( this->pathWriter.c_str(), newName.c_str() ) != 0 ) 
				{
        			std::cout << "Ошибка переименования файла\n";
        			return false;
        		}

        		//DBAccessor::Config cfg = DBAccessor::Config { "localhost", "startup", "start_UP506", "intersection" };
				//DBAccessor::Config cfg = DBAccessor::Config { "localhost", "root", "root", "intersection" };
				//DBAccessor dba(cfg);
				//std::string query = std::string("INSERT INTO `videoclips` (name, duration, d_beg, d_end, cam, cab, isProcessed) VALUES(\"" +
				//newName + "\", \"0\", \""+currentDateTime()+"\", \""+currentDateTime()+"\",\""+this->nameVideoCamName+"\",\""+this->nameVideoCab+"\",0);");
				//int event = dba.execDirect(query);
				//if (event <= 0)
				//{
				//	std::cout<<"ERROR: could not insert event to database.\n"
				//		"Query: " + query +
				//		"\tReturned id: " + std::to_string(event)<<std::endl;
				//}
			}

			this->nameVideoFirstPart=currentDateTime();
		}
	
	/*char c = (char)waitKey(33);
	if (c == 27)
	{
		video.release();
		return true;
	}*/
	return true;
}

Camera::~Camera()
{
	video.release();
	vcap.release();
}

// Get current date/time, format is YYYY-MM-DD|HH:mm:ss
const String Camera::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
    return buf;
}

int Camera::getDuration()
{
	return duration;
}

int Camera::setDuration(int input)
{
	this->duration=input;
}

void Camera::startWrite()
{
	this->canWrite=true;
}

void Camera::stopWrite()
{
	this->canWrite=false;
}

void Camera::setPath(String input)
{
	this->path=input;
}

void Camera::setNameVideoCamName(cv::String input)
{
	this->nameVideoCamName=input;
}

cv::String Camera::getNameVideoCamName()
{
	return this->nameVideoCamName;
}

void Camera::setNameVideoCab(cv::String input)
{
	this->nameVideoCab=input;
}

cv::String Camera::getNameVideoCab()
{
	return this->nameVideoCab;
}

void Camera::Config::parseParams(const std::string& camNumber)
{
	try
	{
		camId = getParamsNew(CAM_CONFIG_NAME, camNumber, { "cam" }).at("cam");
		url = getParamsNew(CAM_CONFIG_NAME, "List", { camId }).at(camId);
	}
	catch (const std::out_of_range& ex)
	{
		std::cout << ex.what() << ": Unable to parse camera id and url." << std::endl;
		return;
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << ": Unknown exception." << std::endl;
		return;
	}
	std::vector<std::string> camParamNames = {
		"office", "sensitivity", "postwrite", "fps", "roi", "rotate", "door"
	};
	try
	{
		auto params = getParamsNew(CAM_CONFIG_NAME, camNumber, camParamNames);
		office = params.at("office");
		sensitivity = stoi(params.at("sensitivity"));
		postwrite = stoi(params.at("postwrite"));
		fps = stod(params.at("fps"));
		auto roiParams = splitNew(params.at("roi"), ',',false);
		detectionRoi = cv::Rect(stoi(roiParams.at(0)), stoi(roiParams.at(1)),
			stoi(roiParams.at(2)), stoi(roiParams.at(3)));
		rotate = stoi(params.at("rotate"));
		door = params.at("door");
	}
	catch (const std::out_of_range& ex)
	{
		std::cout << ex.what() << ": Unable to read some of camera parameters."
			<< "Requested parameters are:" << std::endl;
		std::copy(camParamNames.begin(), camParamNames.end(),
			std::ostream_iterator<std::string>(std::cout, "; "));
		std::cout << std::endl;
		return;
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << ": Unknown exception." << std::endl;
		return;
	}
}

std::map<std::string, std::string>  getParamsNew(const std::string & filename,const std::string & field,const std::vector<std::string> & keys)
{
	std::map<std::string, std::string> params;
	std::ifstream inf(filename);
	if (!inf.is_open())
	{
		std::cout << "Error opening file.\n";
		return params;
	}
	
	std::string line;
	std::getline(inf, line);
	
	while (line.find(std::string("[" + field + "]")) == std::string::npos
		&& inf)
	{
		std::getline(inf, line);
	}
	std::vector<std::string> configStrings;
	std::getline(inf, line);
	while (!std::regex_search(line, std::regex(R"(^\[.*\]$)")) && inf)
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.erase(line.size() - 1, 1);
		}
		if (!line.empty() && line.front() != '#')
		{
			configStrings.push_back(line);
		}
		std::getline(inf, line);
	}
	std::for_each(keys.begin(), keys.end(), [&params, configStrings](std::string key)
	{
		for (auto str : configStrings)
		{
			if (str.substr(0, str.find('=')) == key)
			{
				params[key] = str.substr(str.find('=') + 1);
				break;
			}
		}
	});
	return params;
}

//TODO test this function
std::vector<std::string> splitNew(const std::string & str, char ch, bool skipEmptyStrings)
{
	auto pos = str.find(ch);
	unsigned int initialPos = 0;
	std::vector<std::string> strings;

	// Decompose statement
	while (pos != std::string::npos)
	{
		if (!skipEmptyStrings || initialPos < pos)
		{
			strings.push_back(str.substr(initialPos, pos - initialPos));
		}

		initialPos = pos + 1;
		pos = str.find(ch, initialPos);
	}

	// Add the last one
	if (!skipEmptyStrings || initialPos < str.size())
	{
		strings.push_back(str.substr(initialPos));
	}

	return strings;
}

cv::String Camera::getPath()
{
	return this->path;
}

void Camera::setDrawRectangles(std::vector<cv::Rect> input )
{
	this->rects=input;
}

void Camera::setConfig(Config input)
{
	this->config=input;
}

void Camera::setTimer()
{
	this->timer=time(0);
	this->currentCountOfTimer=0;
}

bool Camera::isTimeUp()
{
	if(this->currentCountOfTimer<=maxDurationVideo)
	{
	std::time_t now=time(0);
	this->currentCountOfTimer=now-this->timer;
	return 0;
	}
	else return 1;
}

void Camera::resetTimer()
{
	this->currentCountOfTimer=0;
}