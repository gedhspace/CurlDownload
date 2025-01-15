
#include <iostream>
#include <curl/curl.h>
#include<string>
#include "Download.h"

#pragma warning(disable : 4996)

using namespace std;


char* strToChar(std::string strSend)
{
	char* ConvertData;
	const int len2 = strSend.length();
	ConvertData = new char[len2 + 1];
	strcpy(ConvertData, strSend.c_str());
	return ConvertData;
}


int main()
{
	Download download;
    
    //download.SetInfo("https://ojproxy.gedh2011.us.kg/https://github.com/gedhspace/CurlDownload/releases/download/1.0/4K.60FPS.6.flv.mp4", "example/image.mp4", GetFileSize("https://ojproxy.gedh2011.us.kg/https://github.com/gedhspace/CurlDownload/releases/download/1.0/4K.60FPS.6.flv.mp4"),64);
    download.SetInfo("https://ojproxy.gedh2011.us.kg/https://github.com/2dust/v2rayN/releases/download/7.5.6/v2rayN-windows-64-With-Core.zip", "example/eaxmple.zip", GetFileSize("https://ojproxy.gedh2011.us.kg/https://github.com/2dust/v2rayN/releases/download/7.5.6/v2rayN-windows-64-With-Core.zip"), 128);
    //download.SetInfo("https://piston-data.mojang.com/v1/objects/5dc770c10d3769f9f3437a2bd7029b10b9c0d780/client.jar", "example/example.jar", GetFileSize("https://piston-data.mojang.com/v1/objects/5dc770c10d3769f9f3437a2bd7029b10b9c0d780/client.jar"), 64);
    download.start();

cout<<"Example is finish."<<endl;
	return 0;
}
/*
#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include <string>

using namespace std;
// 写入数据的回调函数
size_t WriteData(void* ptr, size_t size, size_t nmemb, std::ofstream* stream)
{
    stream->write(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

int download_progress(char* progress_data,
    double t, /* dltotal 
    double d, /* dlnow 
    double ultotal,
    double ulnow) {
    static char bar[120];
    const char* lable = "|/-\\";
    int i = 0;
    if (t) {
        i = d * 100.0 / t;
    }
    printf("%s [%-100s][%d%%][%c]\r", progress_data, bar, i, lable[i % 4]);
    //fflush(stdout);
    bar[i] = '>';
    i++;
    bar[i] = 0;
    return 0;
}

// 分段下载函数
bool DownloadSegment(const std::string& url, std::ofstream& output, long start, long end)
{
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        // 设置URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置Range头
        std::string range = std::to_string(start) + "-" + std::to_string(end);
        curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());

        // 设置写入数据的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress);

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

// 获取文件大小
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

int main() {
    std::string url = "https://img.linux.net.cn/lfs/LFS-BOOK-7.7-systemd.pdf"; // 替换为实际URL
    std::string output_filename = "LFS-BOOK-7.7-systemd.pdf";
    long segment_size = 1024 * 1024; // 1MB

    // 获取文件大小
    long file_size = GetFileSize(url);
    if (file_size == -1) {
        std::cerr << "Failed to get file size." << std::endl;
        return 1;
    }

    std::cout << "File size: " << file_size << " bytes" << std::endl;

    // 打开输出文件
    std::ofstream output(output_filename, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    // 分段下载
    for (long start = 0; start < file_size; start += segment_size) {
        long end = start + segment_size - 1;
        if (end >= file_size) {
            end = file_size - 1;
        }
        std::cout << "Downloading segment: " << start << "-" << end << std::endl;
        if (!DownloadSegment(url, output, start, end)) {
            std::cerr << "Failed to download segment." << std::endl;
            output.close();
            return 1;
        }
    }

    // 关闭输出文件
    output.close();
    std::cout << "Download complete." << std::endl;

    return 0;
}
*/

