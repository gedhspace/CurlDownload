#pragma once
#include <curl/curl.h>
#include<string>
#include <iostream>
#include<thread>
#include <vector>

#pragma warning(disable : 4996)

using namespace std;



class Download {

private:
	string url;
	string filename;
	long long size;
	bool IsSet = false;
	int threadMax=64;
    vector<thread> threads;
public:
	void SetInfo(string surl, string sfilename, long long ssize = 0,int smax=64) {
		url = surl;
		filename= sfilename;
		size= ssize;
		threadMax = smax;
		IsSet = true;
	}
	void DownloadSegment(int begin, int end,int id) {
        
	}
	void ThreadCheck() {
		int nowThread = 0;
        long long begin = 0, end = 0;
        long long segment_size = 1024 ;
		while (true) {
			if (nowThread < threadMax &&threads.size()<=1000) {
                end = begin + segment_size - 1;
                if (end >= size) {
					end = size - 1;
				}
				cout << begin << "-" << end << endl;
				threads.push_back(thread(&Download::DownloadSegment,this, 0, 0, 0));
                threads[threads.size()-1].detach();
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
			cout << "Downloading" << endl;
			while(true){}
		}

	}
};