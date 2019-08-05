#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread> 
#include <filesystem>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <experimental/filesystem>	
#include <stdio.h>
#include <sys/types.h>
#include <windows.h>
#include <conio.h>
#include <unordered_map>
#include <set>
#include <ctime>
#include <chrono>

#define _CRT_SECURE_NO_WARNINGS


using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

std::string getfilename(std::string path);

string image_csv_simulation = "C:\\Users\\kevin\\Desktop\\0724csv_1_simulation";
string pyro_txt_simulation = "C:\\Users\\kevin\\Desktop\\0724pyro_1_simulation";

string date = "2019/07/24";
string image_csv_file = "C:\\Users\\kevin\\Desktop\\0724csv_1";
string pyro_txt_file = "C:\\Users\\kevin\\Desktop\\0724pyro_1";
string sample_position = "C:\\Users\\kevin\\Desktop\\sample_position.csv";

int main() {
	vector<string> pyro_simulation;
	vector<string> image_simulation;
	for (const auto & entry : fs::directory_iterator(image_csv_simulation))
		image_simulation.push_back(entry.path().u8string());
	for (const auto & entry : fs::directory_iterator(pyro_txt_simulation))
		pyro_simulation.push_back(entry.path().u8string());
	sort(pyro_simulation.begin(), pyro_simulation.end());
	sort(image_simulation.begin(), image_simulation.end());
	cout << pyro_simulation[0] << endl;

	vector<vector<vector<double>>> sample_position_csv;

	string line;
	ifstream infile(sample_position);
	getline(infile, line);
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		string d1, d2, t;
		int count = 4;
		vector<vector<double>> tmp;
		getline(iss, d1, ',');
		while (count--) {
			getline(iss, d1, ',');
			getline(iss, d2, ',');
			vector<double> t;
			t.push_back(atof(d1.c_str()));
			t.push_back(atof(d2.c_str()));
			tmp.push_back(t);
		}
		//cout << endl;
		sample_position_csv.push_back(tmp);
	}

	int layer = 0, next_layer = 1, count = 0;
	bool pyro_calculate_features = false, image_calculate_features = false;

	vector<int> check_window_layer(50000, 5000);
	bool state_waitting_layer = true; //record if priting layer
	double wait_time_layer = 0;

	int test = 1;
	while (test--) {
		auto start = chrono::system_clock::now();
		time_t start_time = chrono::system_clock::to_time_t(start);
		cout << put_time(std::localtime(&start_time), "%F %T") << endl;
		int result = 0;
		if (count % 3 == 0) {
			//cout << getfilename(pyro_simulation[0]) << endl;
			result |= rename(image_simulation[count * 3].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3]) + ".csv").c_str());
			result |= rename(image_simulation[count * 3 + 1].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3 + 1]) + ".csv").c_str());
			result |= rename(image_simulation[count * 3 + 2].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3 + 2]) + ".csv").c_str());
		}
		//for(int i=0;i<30;i++)
			//result |= rename((pyro_simulation[count * 30 + i]).c_str(), (pyro_txt_file + "\\"  + getfilename(pyro_simulation[count * 30 + i]) + ".txt").c_str());

		//cout << pyro_txt_file + "\\" + pyro_simulation[0] + ".txt" << endl;
		if (result == 0)
			puts("File successfully renamed");
		else {
			perror("Error renaming file");
			return 0;
		}
		Sleep(1000);

		vector<string> pyro_files;
		vector<string> image_files;
		for (const auto & entry : fs::directory_iterator(image_csv_file))
			image_files.push_back(entry.path().u8string());
		for (const auto & entry : fs::directory_iterator(pyro_txt_file))
			pyro_files.push_back(entry.path().u8string());
		sort(pyro_files.begin(), pyro_files.end());
		sort(image_files.begin(), image_files.end());

		if (count == 0) {
			struct tm tm;
			istringstream  ss(date + "-" + getfilename(pyro_files[0]) + "0");
			ss >> std::get_time(&tm, "%Y/%m/%d-%H_%M_%S_%f");
			time_t pyro_start_time = mktime(&tm);
			cout << put_time(std::localtime(&pyro_start_time), "%F %T") << endl;
		}


	}


	return 0;
}

std::string getfilename(std::string path)
{
	path = path.substr(path.find_last_of("/\\") + 1);
	size_t dot_i = path.find_last_of('.');
	return path.substr(0, dot_i);
}