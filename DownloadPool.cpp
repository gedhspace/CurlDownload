#pragma once

#include "Download.h"
#include <iostream>

using namespace std;


class DownloadPool {

private:
	vector<Download> downloaders;
	int downloading_count = 0;

public:


	void progress_update() {

	}

	void push(std::string surl, std::string sfilename, long long ssize = 0) {
		Download t;
		t.SetInfo(surl, sfilename, ssize);
		downloaders.push_back(t);
	}


	void start() {
		for (auto i : downloaders) {
			i.start();
		}

	}


};