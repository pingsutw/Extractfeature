#include <iostream>
#include <fstream>
#include<stdio.h>
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
#include<sys/types.h>

//#include <boost/filesystem.hpp>
//namespace fs = boost::filesystem;
//#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

string input = "C:\\Users\\kevin\\Desktop\\video1";
string csv_path = input + "\\csv";

void test_func(string path,int csv_index)
{
	auto begin = chrono::high_resolution_clock::now();

	vector<std::string> paths;
	vector<std::string> oldpaths;

	oldpaths = paths;

	std::cout << csv_index << " : "<<path << endl;

	//cv::VideoCapture video("./15_41_24_24670.avi");
	cv::VideoCapture video(path);

	const int cut_size = 12;
	if (!video.isOpened()) {
		std::cout << "Can not find the video!!!" << endl;
		return;
	}

	ofstream csv;
	int found = (int)path.find_last_of("\\");
	int len = path.size() - found - 5;
	string output = csv_path + "\\video-" + path.substr(found+1,len) + ".csv";
	csv.open(output);
	csv << ",Length,Width,X_center,Y_center,\n";

	//Mat merge_frame(240*80, 240*50, CV_8UC3);
	//Mat display(240, 240*3,CV_8U);

	vector<float> W_list(4000, 0);
	vector<float> L_list(4000, 0);
	vector<float> X_center_list(4000, 0);
	vector<float> Y_center_list(4000, 0);
	int width = 0, length = 0, index = 0;
	Mat videoFrame;

	while(true){
		video >> videoFrame;
		if (videoFrame.empty()) {
			break;
		}
		//waitKey(10);
		//imshow("Display window", videoFrame);  

		cvtColor(videoFrame, videoFrame, cv::COLOR_BGR2GRAY);
		GaussianBlur(videoFrame, videoFrame, Size(5, 5), 1, 1);
		int cut_x_start, cut_y_start;
		int cut_x_end, cut_y_end;
		if (index == 0 || L_list[index - 1] == 0 || L_list[index - 1] >= 10) {
			cut_x_start = 0;
			cut_y_start = 0;
			cut_x_end = videoFrame.cols;
			//cout << videoFrame.cols << endl;
			cut_y_end = videoFrame.rows;
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
		//cout<<"type = " << videoFrame.type()<<endl;
		//videoFrame.copyTo(display(Rect(0*240,0*240,240,240)));

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
		//cout << index << endl;
		csv << index << "," << length << "," << width << "," << X_center << "," << Y_center << "," << "\n";
	}
	csv.close();
	//for (const auto & entry : fs::directory_iterator(input)) {
	//	if (entry.path().extension() == ".avi" && find(oldpaths.begin(), oldpaths.end(), entry.path().u8string()) == oldpaths.end()) {
	//		paths.insert(paths.begin(), entry.path().u8string());
	//		oldpaths.insert(oldpaths.begin(), entry.path().u8string());
	//	}

	//}
	//paths.pop_back();

	//auto end = chrono::high_resolution_clock::now();
	//auto dur = end - begin;
	//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	//std::cout << "Time = " << ms / 1000 << " s " << ms % 1000 << " ms" << endl;
}
	


int main(int argc, char* argv[])
{
	int csv_index = 0;
	// check if directory is created or not 
	
	while (input == "") {
		std::cout << "Enter path of video : " << endl;
		cin >> input;
	}

	if (!fs::is_directory(csv_path) || !fs::exists(csv_path)) {
		fs::create_directory(csv_path);
		printf("Directory created\n");
	}
	else {
		printf("Directory exists\n");
		//exit(1);
	}
	vector<std::string> paths;
	for (const auto & entry : fs::directory_iterator(input)) {
		if (entry.path().extension() == ".avi")
			paths.push_back(entry.path().u8string());
	}

	vector<thread> threads(8);
	auto begin = chrono::high_resolution_clock::now();
	int index = 0;
	int threadcnt = 0;

	while (true) {
		for (int i = 0; i < 8; i++) {
			if (index >= paths.size())break;
			threads[i] = thread(test_func, paths[index++],csv_index++);
			threadcnt++;
		}
		for (int i = 0; i < threadcnt; i++) {
			threads[i].join();
		}
		threadcnt = 0;
		if (index >= paths.size())break;
	}


	auto end = chrono::high_resolution_clock::now();
	auto dur = end - begin;
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
	std::cout << "Finally Time = " << ms / 1000 << " s " << ms % 1000 << " ms" << endl;


	//int num_devices = cv::cuda::getCudaEnabledDeviceCount();
	//cout << "Cuda dectect : " << num_devices << endl; 
	//cuda::printCudaDeviceInfo(num_devices);  
	
	
	getchar();
	return 0;
}
