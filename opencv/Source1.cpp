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
#include <numeric>
#include <assert.h>


#define _CRT_SECURE_NO_WARNINGS


using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

string file_prefix = "C:\\Users\\kevin\\Desktop\\";

string image_csv_simulation = file_prefix + "0724csv_1_simulation";
string pyro_txt_simulation = file_prefix + "0724pyro_1_simulation";

string date = "2019/07/24";
string image_csv_file = file_prefix +"0724csv_1";
string pyro_txt_file = file_prefix + "0724pyro_1";
string sample_position_path = file_prefix + "sample_position.csv";

string output = file_prefix + "pyro_layer_time1.txt";
string layer = file_prefix + "layer"; //Need to create dir first 

string AVM_image_features_index[] = { "Length_min","Length_max","Length_mean","Length_var","Length_std","Length_skew",
									   "Length_kurt","Length_1quantile'","Length_2quantile","Length_3quantile","Length_range",
									   "Length_quantile",
									   "Width_min","Width_max","Width_mean","Width_var","Width_std","Width_skew",
									   "Width_kurt","Width_1quantile","Width_2quantile","Width_3quantile","Width_range",
									   "Width_quantile"};

struct Time {
	int year;
	int month;
	int day;
	int hour;
	int min;
	int second;
	int millisecond;

	bool operator < (Time const &t) {
		if (year < t.year)return true;
		if (year == t.year && month < t.month)return true;
		if (year == t.year && month == t.month && day < t.day)return true;
		if (year == t.year && month == t.month && day == t.day && hour < t.hour)return true;
		if (year == t.year && month == t.month && day == t.day && hour == t.hour && min < t.min)return true;
		if (year == t.year && month == t.month && day == t.day && hour == t.hour && min == t.min && second < t.second)return true;
		if (year == t.year && month == t.month && day == t.day && hour == t.hour && min == t.min && second == t.second && millisecond < t.millisecond)return true;
		return false;
	}

};

string getfilename(string path);
ostream & operator << (ostream &out, const Time &c);
void checkTime(Time& t);
double mean(vector<double>& check_window_layer);
void getCurrentTime();
bool isInsidePolygon(vector<double>& pt, vector<vector<double>>& poly);
vector<vector<double>> calculate_image_indicator(vector<vector<double>>& layer_image_feature, vector<int>& Sample_no);
double sum(const vector<double>& numbers);
double computeSampleVariance(const double mean, const std::vector<double>& numbers);
double skewness(vector<double>& arr, double std);
template <typename T1, typename T2> typename T1::value_type quant(const T1 &x, T2 q);
double Kurtosis(vector<double>& arr, double std, double mean);


int main() {
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);

	vector<string> pyro_simulation;
	vector<string> image_simulation;
	for (const auto & entry : fs::directory_iterator(image_csv_simulation))
		image_simulation.push_back(entry.path().u8string());
	for (const auto & entry : fs::directory_iterator(pyro_txt_simulation))
		pyro_simulation.push_back(entry.path().u8string());
	sort(pyro_simulation.begin(), pyro_simulation.end());
	sort(image_simulation.begin(), image_simulation.end());
	//cout << pyro_simulation[0] << endl;

	vector<vector<vector<double>>> sample_position;

	string line;
	ifstream infile(sample_position_path);
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
		sample_position.push_back(tmp);
	}

	int layer = 0, next_layer = 1, count = 0;
	bool pyro_calculate_features = false, image_calculate_features = false;

	vector<double> check_window_layer(50000, 5000);
	bool state_waitting_layer = true; //record if priting layer
	double wait_time_layer = 0;

	vector<vector<double>> layer_image_feature;
	vector<Time> Timetag;

	int test = 1;
	while (true) {
		getCurrentTime();
		int result = 0;
		if (count % 3 == 0) {
			cout << image_simulation[count * 3] << endl;
			cout << image_simulation.size() << endl;
			result |= rename(image_simulation[count * 3].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3]) + ".csv").c_str());
			result |= rename(image_simulation[count * 3 + 1].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3 + 1]) + ".csv").c_str());
			result |= rename(image_simulation[count * 3 + 2].c_str(), (image_csv_file + "\\" + getfilename(image_simulation[count * 3 + 2]) + ".csv").c_str());
		}
		for (int i = 0; i < 30; i++)
			result |= rename((pyro_simulation[count * 30 + i]).c_str(), (pyro_txt_file + "\\" + getfilename(pyro_simulation[count * 30 + i]) + ".txt").c_str());

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
		Time layer_end_time;

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
			while (getline(infile, d1, ',')) {
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

			if (remove(pyro_files[i].c_str()) != 0) {
				perror("Error deleting file");
				return 0;
			}
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
				layer_start_time.millisecond += i * 10;
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
			if ((wait_time_layer > (double)50000) && mean(check_window_layer) < (double)5010) {
				state_waitting_layer = true;
				pyro_calculate_features = true;
				if (layer == next_layer) {
					//write pyro time
					layer_end_time = current_temepr_time;
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

		vector<vector<double>> current_image_feature;

		for (int i = 0; i < image_files.size(); i++) {

			char h[15], m[15], s[16], ms[15];
			istringstream ss(getfilename(image_files[i]));
			sscanf(ss.str().c_str(), "%[^_]_%[^_]_%[^_]_%s", h, m, s, ms);
			string filename_combine = date + ' ' + string(h) + ":"+ string(m) + ":"+ string(s) + '.'+ string(ms) + '0';
			cout << filename_combine << endl;
			Time filetime;
			sscanf(ss.str().c_str(), "%d/%d/%d video-%d:%d:%d.%d", &filetime.year, &filetime.month, &filetime.day,
				&filetime.hour, &filetime.min, &filetime.second, &filetime.millisecond);

			vector<vector<double>> temp_feature;

			ifstream infile(image_files[i]);
			getline(infile, line);
			string d1, d2, t;
			while (getline(infile, line)) {
				istringstream iss(line);
				getline(iss, d1, ',');
				vector<double> t;
				while (getline(iss, d1, ',')) {
					t.push_back(atof(d1.c_str()));
				}
				temp_feature.push_back(t);
				Timetag.push_back(filetime);
			}
			infile.close();
			for (int j = 1; j < temp_feature.size(); j++) {
				Timetag[j].millisecond = Timetag[j - 1].millisecond + 400;
				checkTime(Timetag[i]);
			}
			vector<vector<double>> feature_remove = temp_feature;
			vector<vector<double>> feature_final;
			vector<Time> Timetag_copy;
			for (int j = 0; j < temp_feature.size(); j++) {
				//length and Width 
				if (feature_remove[j][0] < 30 && feature_remove[j][1] < 30) {
					feature_final.push_back(feature_remove[j]);
					Timetag_copy.push_back(Timetag[j]);
				}
			}

			for (int j = 0; j < feature_final.size(); j++) {
				current_image_feature.push_back(feature_final[j]);
			}
			if (state_waitting_layer == false && (layer_end_time < filetime))
				image_calculate_features = true;


			if (remove(image_files[i].c_str()) != 0) {
				perror("Error deleting file");
				return 0;
			}
			else
				puts("File successfully deleted");

		}

		for (int j = 0; j < current_image_feature.size(); j++) {
			layer_image_feature.push_back(current_image_feature[j]);
		}

		count += 1;
		//計算各個樣本特徵
		//sample match and calculate features
		if (pyro_calculate_features && image_calculate_features) {
			getCurrentTime();
			cout << "calculate layer" << layer << " feature" << endl;

			//get sample position
			vector<vector<double>> position;
			for (int i = 0; i < layer_image_feature.size(); i++) {
				//X_center and Y_center
				//mover image position to actual position
				double x = (layer_image_feature[i][2] - 98) * 140 / 75;
				double y = (layer_image_feature[i][3] - 98) * 140 / 93;
				position.push_back({x,y});
			}
			//initial feature sample_no
			vector<int> Sample_no(layer_image_feature.size(), 0);

			//allocate sample number
			for (int i = 0; i < layer_image_feature.size(); i++) {
				for (int j = 0; j < sample_position.size(); j++) {
					bool f = isInsidePolygon(position[i], sample_position[j]);
					if (f == true) {
						layer_image_feature[i].back() = j + 1;
						break;
					}
				}
			}
			//calculate image features
			vector<vector<double>> AVM_image_features = calculate_image_indicator(layer_image_feature,Sample_no);
			ofstream csv;
			csv.open(layer + "\\layer_" + to_string(layer) + "_feature.csv");
			for (int i = 0; i < AVM_image_features_index->size(); i++) {
				csv << AVM_image_features_index[i];
				if (i != AVM_image_features_index->size() - 1)
					csv << ",";
				else
					csv << "\n";
			}
			

			for (auto& feature : AVM_image_features) {
				for (int i = 0; i < feature.size(); i++) {
					csv << feature[i];
					if (i != feature.size() - 1)
						csv << ",";
					else 
						csv << "\n";
				}
			}
			csv.close();
			layer_temper_data.clear();
			pyro_calculate_features = false;
			image_calculate_features = false;
			layer_image_feature.clear();
		}

	}//end of while 


	return 0;
}

string getfilename(string path)
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

void getCurrentTime() {
	auto start = chrono::system_clock::now();
	time_t start_time = chrono::system_clock::to_time_t(start);
	cout << put_time(std::localtime(&start_time), "%F %T") << endl;
}

bool isInsidePolygon(vector<double>& pt, vector<vector<double>>& poly){
	bool f = false;
	int i = -1;
	int l = poly.size();
	int j = l - 1;
	while (i < l - 1) {
		i++;
		if ((poly[i][0] <= pt[0] && pt[0] < poly[j][0]) or (poly[j][0] <= pt[0] && pt[0] < poly[i][0]))
			if (pt[1] < (poly[j][1] - poly[i][1]) * (pt[0] - poly[i][0]) / (poly[j][0] - poly[i][0]) + poly[i][1])
				f = !f;
		j = i;
	}
	return f;
}

double sum(const vector<double>& numbers) {
	double s = 0;
	for (auto& n : numbers)
		s += n;
	return s;
}

double computeSampleVariance(const double mean, const std::vector<double>& numbers){
	if (numbers.size() <= 1u)
		return std::numeric_limits<double>::quiet_NaN();

	auto add_square = [mean](double sum, int i)
	{
		auto d = i - mean;
		return sum + d * d;
	};
	double total = std::accumulate(numbers.begin(), numbers.end(), 0.0, add_square);
	return total / (numbers.size() - 1);
}

double skewness(vector<double>& arr,double std) {
	int n = arr.size();
	// Find skewness using above formula 
	float sum = 0;
	for (int i = 0; i < n; i++)
		sum = (arr[i] - mean(arr)) *(arr[i] - mean(arr)) *(arr[i] - mean(arr));
	return sum / (n * std *std *std *std);
}

template <typename T1, typename T2> typename T1::value_type quant(const T1 &x, T2 q)
{
	assert(q >= 0.0 && q <= 1.0);

	const auto n = x.size();
	const auto id = (n - 1) * q;
	const auto lo = floor(id);
	const auto hi = ceil(id);
	const auto qs = x[lo];
	const auto h = (id - lo);

	return (1.0 - h) * qs + h * x[hi];
}

double Kurtosis(vector<double>& arr, double std, double avg) {
	int k = 0;
	int n = arr.size();
	for(int i=0;i<n;i++)
		k += (arr[i] - avg)*(arr[i] - avg)*(arr[i] - avg)*(arr[i] - avg);
	k = k / (n*std*std*std*std);
	return k;
	k -= 3;
}


// Length, Width, X_center, Y_Center
vector<vector<double>> calculate_image_indicator(vector<vector<double>>& layer_image_feature, vector<int>& Sample_no) {
	vector<vector<double>> AVM_image_features;
	vector<double> temp_sample_L;
	vector<double> temp_sample_W;

	int ma = *max_element(Sample_no.begin(), Sample_no.end());
	for (int i = 0; i < ma; i++) {
		for (int j = 0; j < Sample_no.size(); j++) {
			if (Sample_no[j] == i) {
				temp_sample_L.push_back(layer_image_feature[j][0]);
				temp_sample_W.push_back(layer_image_feature[j][1]);
			}
		}
	}

	double temp_Length_min = *min_element(temp_sample_L.begin(), temp_sample_L.end());
	double temp_Length_max = *max_element(temp_sample_L.begin(), temp_sample_L.end());
	double temp_Length_mean = mean(temp_sample_L);
	double temp_Length_var = computeSampleVariance(temp_Length_mean, temp_sample_L);
	double temp_Length_std = sqrt(temp_Length_var);
	double temp_Length_skew = skewness(temp_sample_L, temp_Length_std);
	double temp_Length_kurt = Kurtosis(temp_sample_L, temp_Length_std, temp_Length_mean);
	double temp_Length_1quantile = quant(temp_sample_L, 0.25);
	double temp_Length_2quantile = quant(temp_sample_L, 0.5);
	double temp_Length_3quantile = quant(temp_sample_L, 0.75);
	double temp_Length_range = temp_Length_max - temp_Length_min;
	double temp_Length_quantile = temp_Length_3quantile - temp_Length_1quantile;


	double temp_Width_min = *min_element(temp_sample_W.begin(), temp_sample_W.end());
	double temp_Width_max = *max_element(temp_sample_W.begin(), temp_sample_W.end());
	double temp_Width_mean = mean(temp_sample_W);
	double temp_Width_var = computeSampleVariance(temp_Width_mean, temp_sample_W);
	double temp_Width_std = sqrt(temp_Width_var);
	double temp_Width_skew = skewness(temp_sample_W, temp_Width_std);
	double temp_Width_kurt = Kurtosis(temp_sample_W, temp_Width_std, temp_Width_mean);
	double temp_Width_1quantile = quant(temp_sample_W, 0.25);
	double temp_Width_2quantile =  quant(temp_sample_W, 0.50);
	double temp_Width_3quantile = quant(temp_sample_W, 0.75);
	double temp_Width_range= temp_Width_max - temp_Width_min;
	double temp_Width_quantile = temp_Width_3quantile - temp_Width_1quantile;

	AVM_image_features.push_back({ temp_Length_min, temp_Length_max, temp_Length_mean, temp_Length_var, temp_Length_std,
					temp_Length_skew, temp_Length_kurt, temp_Length_1quantile, temp_Length_2quantile,temp_Length_3quantile,
					temp_Length_range, temp_Length_quantile,
					temp_Width_min, temp_Width_max, temp_Width_mean, temp_Width_var, temp_Width_std, temp_Width_skew,
					temp_Width_kurt, temp_Width_1quantile, temp_Width_2quantile, temp_Width_3quantile, temp_Width_range,
					temp_Width_quantile });

	return AVM_image_features;
}
