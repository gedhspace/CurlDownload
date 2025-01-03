#pragma once
#include <curl/curl.h>
#include<string>
#include <iostream>
#include<thread>

#pragma warning(disable : 4996)

using namespace std;

class Download {

private:
	string url;
	string filename;
	long long size;
	bool IsSet = false;
	int threadMax=64;
public:
	void SetInfo(string surl, string sfilename, long long ssize = 0,int smax=64) {
		url = surl;
		filename= sfilename;
		size= ssize;
		threadMax = smax;
		IsSet = true;
	}
	void download(int begin, int end) {

	}
	void ThreadCheck() {
		int nowThread = 0;
		while (true) {
			if (nowThread < threadMax) {
				thread(this, download, 0, 0).detach();
				nowThread++;
			}
		}
	}
	void start() {
		if (!IsSet) {
			cout << "Please set the download info first!" << endl;
		}
		else {
			thread(this,ThreadCheck).detach();
			cout << "Downloading" << endl;
		}

	}
};