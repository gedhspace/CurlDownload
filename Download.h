#pragma once
#include <curl/curl.h>
#include<string>
#include <iostream>
#include<thread>
#include <cstdio>
#include <vector>
#include<queue>
#include<map>
#include <fstream>
#include<windows.h>
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR '\\'

#pragma warning(disable : 4996)

using namespace std;

long GetFileSize(const std::string& url)
{
	CURL* curl;
	CURLcode res;
	double file_size = 0.0;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 不下载内容
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L); // 只获取头部信息
		// 设置SSL选项
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 验证服务器的SSL证书
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // 验证证书上的主机名
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &file_size);
			if ((res == CURLE_OK) && (file_size > 0.0)) {
				curl_easy_cleanup(curl);
				return static_cast<long>(file_size);
			}
		}
		else {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	return -1;
}

size_t WriteData(void* ptr, size_t size, size_t nmemb, std::ofstream* stream)
{
	stream->write(static_cast<char*>(ptr), size * nmemb);
	//cout << ptr << endl;
	return size * nmemb;
}

bool mergeFiles(const std::string& file1, const std::string& file2) {
	std::ifstream inFile1(file1, std::ios::binary);
	std::ifstream inFile2(file2, std::ios::binary);
	std::ofstream outFile(file1, std::ios::binary | std::ios::app);
	if (!inFile1.is_open()) {
		cout << "Merge:Cannot open the files(inFile1)." << endl;

		return false;
	}
	if (!inFile2.is_open()) {
		cout << "Merge:Cannot open the files(inFile2)." << endl;
		return false;
	}
	if (!outFile.is_open()) {
		cout << "Merge:Cannot open the files(outFile)." << endl;
		return false;
	}
	// 将第二个文件的内容追加到第一个文件
	outFile << inFile2.rdbuf();
	inFile1.close();
	inFile2.close();
	outFile.close();
	return true;
}

void rmfile(char path[]) {
	for (int i = 0; path[i]; i++) {
		if (path[i] == '/') {
			path[i] = PATH_SEPARATOR;
		}
	}
	char mkdir_command[1024];
	sprintf(mkdir_command, "del %s", path);
	system(mkdir_command);
}
char* appedd(const char* a, const char* b) {
	const char* str1 = a;
	const char* str2 = b;
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	size_t new_length = len1 + len2 + 1;
	char* result = new char[new_length];
	strcpy(result, str1);
	strcat(result, str2);
	return result;
}


/*
struct progressData {
	long long progress;
	bool isend = false;
	int id;
};
int progressCallback(void* clientp,
	double dltotal,
	double dlnow,
	double ultotal,
	double ulnow) {

	progressData* data = static_cast<progressData*>(clientp);
	data->progress = static_cast<int>(dlnow);
	//cout << data->progress << endl;;

	return 0;
}
*/
class Download {

private:
	std::string url;
	std::string filename;
	long long size;
	bool IsSet = false;
	int threadMax = 64;
	std::map<int, thread> threads;
	bool isok;
public:
	long long nowd = 0;
	vector<int> threadEnd;
	vector<long long> progr;
	map<int, long long> dlast;
	long long segment_size = 1024 * 1024*3;//3MB
	int nowThread = 0;
	int oksum = 0;

	void SetInfo(std::string surl, std::string sfilename, long long ssize = 0, int smax = 64) {
		url = surl;
		filename = sfilename;
		size = ssize;
		threadMax = smax;
		IsSet = true;
	}
	/*
	void progress_check(progressData* Data1) {
		while (true) {
			//int isend = Data1->isend;
			cout << endl;
			
			if (Data1->isend==true) {
				//cout <<"is end." << endl;
				break;
			}
			int out = Data1->progress;
			int id = Data1->id;
			progr.push_back(out - dlast[id]);
			//cout << "add "<<out - dlast[id] << endl;
			dlast[id] = out;
		}

	}*/
	bool DownloadSegment(long start, long end, int id) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::ofstream output(filename + "." + std::to_string(id) + ".CurlDownload", std::ios::binary);
		//cout << filename + "." + to_string(id) + ".CurlDownload" << endl;
		//cout << start << "-" << end << endl;

		if (!output.is_open()) {
			std::cerr << "Failed to open output file." << std::endl;
			return 1;
		}

		CURL* curl;
		CURLcode res;
		curl = curl_easy_init();
		if (curl) {
			// 设置URL
			//cout << url << endl;

			//progressData data;
			//data.id = id;

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			// 设置Range头
			std::string range = std::to_string(start) + "-" + std::to_string(end);
			curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());

			// 设置写入数据的回调函数
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

			// 设置SSL选项
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 验证服务器的SSL证书
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // 验证证书上的主机名

			//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			//curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
			//curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &data);

			//thread(&Download::progress_check, this, &data).detach();
			// 执行请求
			res = curl_easy_perform(curl);

			// 清理
			curl_easy_cleanup(curl);

			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
				return false;
			}
			else {
				//printf("Download progress: %d%%\n", data.progress); // 使用回调中更新的进度信息
			}
			output.close();
			//data.isend = true;
			threadEnd.push_back(id);
			//cout << id<<"return" << endl;
			Sleep(500);
			return true;
		}
		return false;
	}
	void displayProgressBar(int progress,long long speed,int nowthreadpr) {
		int barWidth = 70;
		std::cout << "[";
		int pos = barWidth * progress / 100;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << progress << "% speed: " << (speed / 1024/1024) << " MB Running:"<<nowthreadpr<<" \r";
		std::cout.flush();
	}

	void ThreadCheck() {
		threadEnd.clear();
		//thread(&Download::TimeCheck,this).detach();
		//thread(&Download::Timecount, this).detach();

		long long begin = 0, end = 0;
		//cout << size << endl;
		if (size == -1) {
			cout << "Error: Size is -1." << endl;
			exit(0);
		}
		int nowID = 1;
		bool last = false;
		long long lastp = 0;
		int sum = 0;
		if (size % segment_size == 0) {
			sum = size / segment_size;
		}
		else {
			sum = (size / segment_size) + 1;
		}
		cout << "Number of segments:" << sum << endl;
		//goto merge;
		while (true) {
			if (threads.size() < threadMax && !last) {
				end = begin + segment_size - 1;
				if (end >= size) {
					end = size - 1;
					last = true;
					//cout << end << endl;
				}
				//cout << begin << "-" << end << endl;
				threads[nowID] = thread(&Download::DownloadSegment, this, begin, end, nowID);
				dlast[nowID] = 0;
				threads[nowID].detach();
				//threadsTime[nowID] = 0;
				nowID++;
				begin += segment_size;
				nowThread++;
			}
			while (!threadEnd.empty()) {
				if (!threadEnd.empty()) {
					oksum++;
					nowThread--;
					int rmid = threadEnd[0];
					//cout << rmid << endl;
					//cout << threadEnd[0] << " is end.";
					
					if (threadEnd[0] != sum) {
						nowd += segment_size;
					}
					else {
						nowd += size % segment_size;
					}
					//cout<< "Thread " << rmid << " is end.Use"<<threadsTime[rmid]<<"s." << endl;
					threads.erase(rmid);
					//threadsTime.erase(rmid);
					threadEnd.erase(threadEnd.begin());

				}
			}
			/*
			while (!progr.empty()) {
				//cout << progr.size() << endl;
				long long add = progr[0];
				//cout<<progr.front() <<endl;
				nowd += add;
				progr.erase(progr.begin());
			}
*/
			displayProgressBar(int(nowd * 1.0 / size * 1.0 * 100),nowd-lastp,nowThread);
			lastp = nowd;

			//displayProgressBar(int(nowd * 1.0 / size * 1.0 * 100));
			//cout << nowd * 1.0 / size * 1.0 * 100<<"%" << endl;

			if (oksum >= sum) {
				break;
			}
			Sleep(500);
		}
		Sleep(200);
		displayProgressBar(int(nowd * 1.0 / size * 1.0 * 100), nowd - lastp,nowThread);
	merge:
		
		cout << "Merge file." << endl;
		//nowID = 3;
		for (int i = 1; i <= nowID - 1; i++) {
			cout << filename + "." + to_string(i) + ".CurlDownload" << endl;

			string next = filename + "." + to_string(i) + ".CurlDownload";
			if (mergeFiles(filename, next) == false) {
				cout << "Merge Error." << endl;
				//exit(0);
				goto merge;
			}
		}

	remove:
		for (int i = 1; i <= nowID - 1; i++) {
			cout << filename + "." + to_string(i) + ".CurlDownload" << endl;

			string next = filename + "." + to_string(i) + ".CurlDownload";
			rmfile(appedd(next.c_str(), ""));

		}
		//cout << "Download Finish." << endl;
		isok = true;

	}

	void start() {

		if (!IsSet && size <= 0) {
			cout << "Please set the download info first!" << endl;
		}
		else {
			isok = false;
			thread t(&Download::ThreadCheck, this);
			t.detach();
			long long time = 0;
			//DownloadSegment(0, 255, 1);
			cout << "Downloading" << endl;

			while (true) {
				if (isok) {
					break;
				}
				Sleep(1000);
				time++;
			}

			cout << "Download Finish.Use " << time << "s." << endl;

		}
	}
};