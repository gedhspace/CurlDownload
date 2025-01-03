#include <iostream>
#include <curl/curl.h>
#include "Download.h"

#pragma warning(disable : 4996)

char* strToChar(std::string strSend)
{
	char* ConvertData;
	const int len2 = strSend.length();
	ConvertData = new char[len2 + 1];
	strcpy(ConvertData, strSend.c_str());
	return ConvertData;
}

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

int main()
{
	Download download;
    download.SetInfo("https://bkimg.cdn.bcebos.com/pic/242dd42a2834349b00251014cfea15ce36d3be7b", "image.png", GetFileSize("https://bkimg.cdn.bcebos.com/pic/242dd42a2834349b00251014cfea15ce36d3be7b"),128);
    download.start();
}