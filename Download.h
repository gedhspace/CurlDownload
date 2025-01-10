#pragma once
#include <curl/curl.h>
#include<string>
#include <iostream>
#include<thread>
#include <vector>
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
	cout << ptr << endl;
	return size * nmemb;
}

class Download {

private:
	string url;
	string filename;
	long long size;
	bool IsSet = false;
	int threadMax=64;
    vector<thread> threads;
	bool isEnd[5000];
public:
	
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
		cout << filename + "." + to_string(id) + ".CurlDownload" << endl;
		cout << start << "-" << end << endl;

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
			return true;
		}
		return false;
	}
	void ThreadCheck() {
		memset(isEnd, false, sizeof(isEnd));
		int nowThread = 0;
        long long begin = 0, end = 0;
        long long segment_size = 1024*1024;
		int nowID = 1;
		bool last = false;
		while (true) {
			if (nowThread < threadMax &&threads.size()<=1000) {
			
                end = begin + segment_size - 1;
                if (end >= size) {
					end = size - 1;
					last = true;
					cout << end << endl;
				}
				cout << begin << "-" << end << endl;
				threads.push_back(thread(&Download::DownloadSegment,this, begin, end, nowID));
				
                threads[threads.size()-1].detach();
				
				nowID++;
                begin+=segment_size;
				nowThread++;
			}
			
		}
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