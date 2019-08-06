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


using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

#define max_worker  3
#define max_frame  3000

bool remove_video = true;
string input = "C:\\Users\\kevin\\Desktop\\video1";
string csv_path = input + "\\csv";
unordered_map<string, int> paths;
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

void Extract_feature(string path)
{
	cv::VideoCapture video(path);
	const int cut_size = 30;
	if (!video.isOpened()) {
		//std::cout << "Can not find the video!!!" << endl;
		return;
	}

	ofstream csv;
	int found = (int)path.find_last_of("\\");
	int len = path.size() - found - 5;
	string output = csv_path + "\\video-" + path.substr(found + 1, len) + ".csv";
	csv.open(output);
	csv << ",Length,Width,X_center,Y_center,\n";

	vector<float> W_list(max_frame, 0);
	vector<float> L_list(max_frame, 0);
	vector<float> X_center_list(max_frame, 0);
	vector<float> Y_center_list(max_frame, 0);
	int width = 0, length = 0, index = 0;
	Mat videoFrame;

	while (true) {
		video >> videoFrame;
		if (videoFrame.empty())
			break;
		
		cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2GRAY);
		GaussianBlur(videoFrame, videoFrame, Size(5, 5), 1, 1);
		int cut_x_start, cut_y_start;
		int cut_x_end, cut_y_end;
		if (index == 0 || L_list[index - 1] == 0 || L_list[index - 1] >= cut_size - 2) {
			cut_x_start = 0;
			cut_y_start = 0;
			cut_x_end = videoFrame.cols;
			cut_y_end = videoFrame.rows;
		}
		else {
			int half_cut_size = cut_size / 2;
			cut_x_start = (int)max(0.0f, round(X_center_list[index - 1] - half_cut_size));
			cut_y_start = (int)max(0.0f, round(Y_center_list[index - 1] - half_cut_size));
			cut_x_end = (int)min((float)videoFrame.cols, round(X_center_list[index - 1] + half_cut_size));
			cut_y_end = (int)min((float)videoFrame.rows, round(Y_center_list[index - 1] + half_cut_size));
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
			width = length = X_center = Y_center = 0;
		}
		W_list[index] = width;
		L_list[index] = length;
		X_center_list[index] = X_center;
		Y_center_list[index] = Y_center;
		index++;
		csv << index << "," << length << "," << width << "," << X_center << "," << Y_center << "," << "\n";
	}
	csv.close();
	paths[path]++;
	if (index > 2000 || paths[path] == 3) {
		if (remove_video)
			removed.insert(path);
		else
			visited.insert(path);
		paths.erase(path);
	}
}

int main1(int argc, char* argv[])
{
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);

	string process = "opencv.exe";
	SetPriorityClass(&process, REALTIME_PRIORITY_CLASS);
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	while (input == "") {
		std::cout << "Enter path of video : " << endl;
		cin >> input;
	}
	// check csv directory is created or not 
	if (!fs::exists(csv_path)) {
		fs::create_directory(csv_path);
		printf("CSV directory created\n");
	}
	else if(!fs::is_directory(csv_path)) {
		printf("Can not create directory\n");
		return 0;
	}
	else {
		printf("CSV directory exists\n");
	}
	for (const auto & entry : fs::directory_iterator(input)) {
		if (entry.path().extension() == ".avi") {
			paths[entry.path().u8string()] = 0;
		}
	}

	int workder_cnt = 0, index = 0;
	vector<thread> workers(max_worker);
	thread listener = thread(keyboard_input);
	auto begin = chrono::high_resolution_clock::now();
	
	while (true) {
		for (auto path : paths) {
			if (workder_cnt == max_worker)
				break;
			workers[workder_cnt++] = thread(Extract_feature, path.first);
			printf("[ %d ] %s\n", index++, path.first.c_str());
		}
		for (int k = 0; k < workder_cnt; k++) {
			workers[k].join();
		}
		workder_cnt = 0;
		for (auto path : removed) {
			if (remove(path.c_str()) != 0) {
				perror("Error deleting file");
			}
			else {
				removed.erase(path);
				printf("%s already deleted !!\n", path.c_str());
			}
		}
		if (paths.empty()) {
			printf("Please add new video !!\n");
			Sleep(max_worker*1000);
			for (const auto & entry : fs::directory_iterator(input)) {
				auto p = entry.path();
				if (p.extension() == ".avi" && !visited.count(p.u8string()))
					paths[p.u8string()] = 0;
			}
		}
		if (remove_video) {
			for (auto path : visited) {
				if (!remove_video)
					break;
				removed.insert(path);
				visited.erase(path);
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
