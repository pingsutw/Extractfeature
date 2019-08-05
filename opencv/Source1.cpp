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



string image_csv_simulation = "C:\\Users\\kevin\\Desktop\\0724csv_1_simulation";
string pyro_txt_simulation = "C:\\Users\\kevin\\Desktop\\0724pyro_1_simulation";

string date = "2019/07/24";
string image_csv_file = "C:\\Users\\kevin\\Desktop\\0724csv_1";
string pyro_txt_file = "C:\\Users\\kevin\\Desktop\\0724pyro_1";
string sample_position = "C:\\Users\\kevin\\Desktop\\sample_position.csv";

string output = "C:\\Users\\kevin\\Desktop\\pyro_layer_time1.txt";

struct Time {
	int year;
	int month;
	int day;
	int hour;
	int min;
	int second;
	int millisecond;
};

string getfilename(string& path);
ostream & operator << (ostream &out, const Time &c);
void checkTime(Time& t);
double mean(vector<double>& check_window_layer);

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
	while (getline(infile, line))
	{
		istringstream iss(line);
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

	vector<double> check_window_layer(50000, 5000);
	bool state_waitting_layer = true; //record if priting layer
	double wait_time_layer = 0;

	int test = 1;
	unordered_map<string, unordered_map<int, int>> layer_image_feature;
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
		for(int i=0;i<30;i++)
			result |= rename((pyro_simulation[count * 30 + i]).c_str(), (pyro_txt_file + "\\"  + getfilename(pyro_simulation[count * 30 + i]) + ".txt").c_str());

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
		Time current_temepr_time;

		if (count == 0) {
			struct tm tm;
			cout << pyro_files[0] << endl;
			istringstream  ss(date + "-" + getfilename(pyro_files[0]) + "0");
			Time pyro_start_time;
			
			sscanf(ss.str().c_str(), "%d/%d/%d-%d_%d_%d_%d", &pyro_start_time.year, &pyro_start_time.month, &pyro_start_time.day,
				&pyro_start_time.hour, &pyro_start_time.min, &pyro_start_time.second, &pyro_start_time.millisecond);

			current_temepr_time = pyro_start_time;
			cout << ss.str() << endl;

			Time layer_end_time = pyro_start_time;
			layer_end_time.day + 1;
			checkTime(layer_end_time);

			ofstream csv;
			csv.open(output);
			csv << "Pyro start at " << pyro_start_time << "\n";
			csv.close();
		}
		vector<double> layer_temper_data;
		vector<double> current_temper_data;

		//讀取溫度資料，轉換成溫度值，並串聯起來
		//read pyro data, calculate temperature and concate
		for (int i = 0; i < pyro_files.size(); i++) {
			istringstream infile(pyro_files[i]);
			string d1, d2;
			vector<vector<double>> temp_array;
			while (getline(infile, d1,',')) {
				getline(infile, d2, ',');
				vector<double> t;
				if (d2 == "")break;
				t.push_back(atof(d1.c_str()));
				t.push_back(atof(d2.c_str()));
				temp_array.push_back(t);
			}
			vector<double> temp_temper(temp_array.size());
			for (int j = 0; j < temp_temper.size(); j++) {
				temp_temper[i] = ((temp_array[i][0] * pow(2, 8) + temp_array[i][1]) * 0.061257618916352723 + 492.77160096787043) * 10;
				if (temp_temper[i] > 4928)
					current_temper_data.push_back(temp_temper[i]);
			}
			
			if (remove(pyro_files[i].c_str()) != 0)
				perror("Error deleting file");
			else
				puts("File successfully deleted");
		}
		for (auto& d : current_temper_data) {
			layer_temper_data.push_back(d);
		}
		/*利用moving windows尋找每層開始以及結束時間點
			check the layer segment point
			current_temper_data = np.array(current_temper_data, dtype = float)*/
		for (int i = 0; i < current_temper_data.size(); i++) {
			double temp_temper = current_temper_data[i];
			if (state_waitting_layer && (temp_temper > 5020)) {
				layer += 1;
				state_waitting_layer = false;
				wait_time_layer = 0;

				//write pyro time
				Time layer_start_time = current_temepr_time;
				layer_start_time.millisecond += i* 10;
				checkTime(layer_start_time);
				ofstream csv;
				csv.open(output);
				csv << "Layer " << layer << " start at " << layer_start_time << "\n";
				csv.close();
			}

			for (int j = 0; j < check_window_layer.size() - 1; j++) {
				check_window_layer[j] = check_window_layer[j + 1];
				check_window_layer.back() = temp_temper;
			}
			if (state_waitting_layer == false) {
				wait_time_layer += 1;
			}
			//double mean = accumulate(check_window_layer.begin(), check_window_layer.end(), 0.0) / check_window_layer.size();
			double d;
			if ((wait_time_layer > (double)50000)) {
				if (mean(check_window_layer) < (double)5010)continue;
				state_waitting_layer = true;
				pyro_calculate_features = true;
				if (layer == next_layer) {
					//write pyro time
					Time layer_end_time = current_temepr_time;
					layer_end_time.millisecond += i * 10;
					checkTime(layer_end_time);

					ofstream csv;
					csv.open(output);
					csv << "Layer " << layer << " end at " << layer_end_time << "\n";
					csv.close();
					next_layer += 1;
				}
			}
		}
		current_temepr_time.millisecond += current_temper_data.size() * 10;
		checkTime(current_temepr_time);
		//讀取圖像資料，並串聯起來
		//read image data and concate

		unordered_map<string, unordered_map<int, int>> current_image_feature;
		
		for (int i = 0; i < image_files.size(); i++) {
			//istringstream infile(image_files[i]);
			istringstream  ss(getfilename(image_files[i]));
			vector<string> filename_split(4);

			sscanf(ss.str().c_str(), "%s_%s_%s_%s", filename_split[0], filename_split[1], filename_split[2], filename_split[3]);
			string filename_combine = date + ' ' + filename_split[0] + ':' + filename_split[1] + ':' +
				filename_split[2] + '.' + filename_split[3] + '0';
			cout << filename_combine << endl;

			string d1, d2;
			vector<vector<double>> temp_array;
			while (getline(infile, d1, ',')) {
				getline(infile, d2, ',');
				vector<double> t;
				if (d2 == "")break;
				t.push_back(atof(d1.c_str()));
				t.push_back(atof(d2.c_str()));
				temp_array.push_back(t);
			}
		}



	}//end of while 


	return 0;
}

string getfilename(string& path)
{
	path = path.substr(path.find_last_of("/\\") + 1);
	size_t dot_i = path.find_last_of('.');
	return path.substr(0, dot_i);
}

double mean(vector<double>& check_window_layer) {
	double sum = 0;
	for (auto & layer : check_window_layer) {
		sum += layer;
	}
	return sum / check_window_layer.size();
}

void checkTime(Time& t) {
	t.second = t.millisecond % 1000000;
	t.min = t.second % 60;
	t.hour = t.min % 60;
	t.day = t.hour % 24;
	t.month = t.day % 30;
	t.year = t.month % 12;
}

ostream & operator << (ostream &out, const Time &c)
{
	out << c.year << "/" << c.month << "/" << c.day << "-" << c.hour << ":" << c.min << ":" << c.second << "." << c.millisecond;
	return out;
}