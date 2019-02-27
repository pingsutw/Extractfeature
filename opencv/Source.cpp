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

//#include <boost/filesystem.hpp>
//namespace fs = boost::filesystem;
//#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

string input = "C:\\Users\\kevin\\Desktop\\video1";
string csv_path = input + "\\csv";
bool remove_video = true;
set<string> paths;
set<string> visited;
set<string> removed;

void keyboard_input() {
	while (int key = _getch()) {
		if (key == VK_ESCAPE) {
			if (remove_video) {
				cout << "remove_video turn off" << endl;
				remove_video = false;
			}
			else {
				cout << "remove_video turn on" << endl;
				remove_video = true;
			}
		}
	}
}

void Extract_feature(string path, int csv_index)
{
	cout << csv_index << " : " << path << endl;
	cv::VideoCapture video(path);

	const int cut_size = 30;
	if (!video.isOpened()) {
		std::cout << "Can not find the video!!!" << endl;
		return;
	}

	ofstream csv;
	int found = (int)path.find_last_of("\\");
	int len = path.size() - found - 5;
	string output = csv_path + "\\video-" + path.substr(found + 1, len) + ".csv";
	csv.open(output);
	csv << ",Length,Width,X_center,Y_center,\n";

	vector<float> W_list(3000, 0);
	vector<float> L_list(3000, 0);
	vector<float> X_center_list(3000, 0);
	vector<float> Y_center_list(3000, 0);
	int width = 0, length = 0, index = 0;
	Mat videoFrame;

	while (true) {
		video >> videoFrame;
		if (videoFrame.empty()) {
			break;
		} 

		cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2GRAY);
		GaussianBlur(videoFrame, videoFrame, Size(5, 5), 1, 1);
		int cut_x_start, cut_y_start;
		int cut_x_end, cut_y_end;
		if (index == 0 || L_list[index - 1] == 0 || L_list[index - 1] >= cut_size - 2) {
			cut_x_start = 0;
			cut_y_start = 0;
			cut_x_end = 0;
			cut_y_end = 0;
		}
		else {
			int half_cut_size = cut_size / 2;
			cut_x_start = (int)max(0.0f, round(X_center_list[index - 1] - half_cut_size));
			cut_y_start = (int)max(0.0f, round(Y_center_list[index - 1] - half_cut_size));
			cut_x_end = (int)min((float)videoFrame.cols, round(X_center_list[index - 1] + half_cut_size));
			cut_y_end = (int)min((float)videoFrame.rows, round(Y_center_list[index - 1] + half_cut_size));
			//Rect myROI(cut_y_start, cut_x_start, cut_y_end - cut_y_start, cut_x_end - cut_x_start);
			//videoFrame = videoFrame(myROI);
		}
		Canny(videoFrame, videoFrame, 80, 160);

		int minX = 9999, maxX = 0, minY = 9999, maxY = 0;

		for (int i = cut_y_start; i < cut_y_end - 1; i++) {
			for (int j = cut_x_start; j < cut_x_end - 1; j++) {
				if (videoFrame.at<uchar>(i, j) == 255) {
					minX = min(minX, j);
					maxX = max(maxX, j);
					minY = min(minY, i);
					maxY = max(maxY, i);
				}
			}
		}
		width = maxX - minX;
		length = maxY - minY;

		float X_center = (maxX + minX) / 2.0f;
		float Y_center = (maxY + minY) / 2.0f;

		if (minX == 9999) {
			width = 0;
			length = 0;
			X_center = 0;
			Y_center = 0;
		}
		W_list[index] = width;
		L_list[index] = length;
		X_center_list[index] = X_center;
		Y_center_list[index] = Y_center;
		index++;
		csv << index << "," << length << "," << width << "," << X_center << "," << Y_center << "," << "\n";
	}
	csv.close();
	if (index > 2000) {
		if (remove_video)
			removed.insert(path);
		else
			visited.insert(path);
		paths.erase(path);
	}
}



int main(int argc, char* argv[])
{
	int csv_index = 0;

	while (input == "") {
		std::cout << "Enter path of video : " << endl;
		cin >> input;
	}
	// check csv directory is created or not 
	if (!fs::is_directory(csv_path) || !fs::exists(csv_path)) {
		fs::create_directory(csv_path);
		printf("Directory created\n");
	}
	else {
		printf("Directory exists\n");
	}
	for (const auto & entry : fs::directory_iterator(input)) {
		if (entry.path().extension() == ".avi") {
			paths.insert(entry.path().u8string());
		}
	}

	int worker_max = 3;
	int workder_cnt = 0;
	vector<thread> workers(worker_max);
	thread listener = thread(keyboard_input);
	auto begin = chrono::high_resolution_clock::now();
	int index = 0;
	
	

	while (true) {
		int i = 0;
		for (auto path : paths) {
			if (workder_cnt == worker_max)
				break;
			workers[workder_cnt++] = thread(Extract_feature, path, csv_index++);
		}
		for (int k = 0; k < workder_cnt; k++) {
			workers[k].join();
			for (auto path : removed) {
				if (remove(path.c_str()) != 0) {
					perror("Error deleting file");
				}
				else {
					removed.erase(path);
					puts("File successfully deleted");
				}
			}
		}
		workder_cnt = 0;
		if (remove_video) {
			for (auto path : visited) {
				if (!remove_video)
					break;
				removed.insert(path);
				visited.erase(path);
			}
		}
		if (paths.empty()) {
			printf("Don't have video !!\n");
			Sleep(4000);
			for (const auto & entry : fs::directory_iterator(input)) {
				auto p = entry.path();
				if (p.extension() == ".avi" && !visited.count(p.u8string()))
					paths.insert(p.u8string());
			}
		}
	}

	auto end = chrono::high_resolution_clock::now();
	auto dur = end - begin;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	cout << "Finally Time = " << ms / 1000 << " s " << ms % 1000 << " ms" << endl;

	getchar();
	return 0;
}
