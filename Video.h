#pragma once
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <sys/stat.h>
#include <map>
#include <regex>
#define MIN_LEN 30



class Video
{
public:
	Video();
	~Video();
	static void createMainVideoFolder();	
	static void createDateVideoFolder();	
};