#pragma once
#include <curl/curl.h>
#include<string>
#include <iostream>
#include<thread>
#include <vector>
#include<map>
#include <fstream>

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
	if (!inFile1.is_open() ) {
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

class Download {

private:
	string url;
	string filename;
	long long size;
	bool IsSet = false;
	int threadMax=64;
    map<int,thread> threads;
	
public:
	vector<int> threadEnd;
	void SetInfo(string surl, string sfilename, long long ssize = 0,int smax=64) {
		url = surl;
		filename= sfilename;
		size= ssize;
		threadMax = smax;
		IsSet = true;
	}
	bool DownloadSegment(long start, long end,int id){
		this_thread::sleep_for(std::chrono::seconds(5));
		ofstream output(filename + "." + to_string(id) + ".CurlDownload", std::ios::binary);
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
			cout << url << endl;
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

			// 执行请求
			res = curl_easy_perform(curl);

			// 清理
			curl_easy_cleanup(curl);

			if (res != CURLE_OK) {
				std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
				return false;
			}
			output.close();
			threadEnd.push_back(id);
			return true;
		}
		return false;
	}
	void ThreadCheck() {
		threadEnd.clear();
		std::ofstream output(filename, std::ios::binary);
		int oksum = 0;
		int nowThread = 0;
        long long begin = 0, end = 0;
        long long segment_size = 1024*10;
		int nowID = 1;
		bool last = false;
		int sum = 0;
		if (size % segment_size == 0) {
			sum = size / segment_size;
		}
		else {
			sum = (size / segment_size) + 1;
		}
		cout << "Number of segments:" << sum << endl;
		
		while (true) {
			if (threads.size()<threadMax&&!last) {
                end = begin + segment_size - 1;
                if (end >= size) {
					end = size - 1;
					last = true;
					//cout << end << endl;
				}
				cout << begin << "-" << end << endl;
				threads[nowID]=thread(&Download::DownloadSegment, this, begin, end, nowID);
                threads[nowID].detach();
				nowID++;
                begin+=segment_size;
				nowThread++;
			}
			while (!threadEnd.empty()) {
				if (!threadEnd.empty()) {
					oksum++;
					nowThread--;
					cout << threadEnd[0]<<" is end.";
					threads.erase(threadEnd[0]);
					threadEnd.erase(threadEnd.begin());
				}
			}
			
			if (oksum >= sum) {
				break;
			}
		}
		Sleep(2000);
		cout << "Merge file." << endl;
		
		for (int i = 1; i <= nowID-1; i++) {
			cout<<filename + "." + to_string(i) + ".CurlDownload"<<endl;
			string next = filename + "." + to_string(i) + ".CurlDownload";
			if (mergeFiles(filename, next) == false) {
				cout << "Merge Error." << endl;
				exit(0);
			}
		}
		cout<<"Download Finish."<<endl;
	}
	void start() {
		if (!IsSet) {
			cout << "Please set the download info first!" << endl;
		}
		else {
			thread t(&Download::ThreadCheck, this);
			t.detach();
			//DownloadSegment(0, 255, 1);
			cout << "Downloading" << endl;

			while(true){}
		}

	}
};