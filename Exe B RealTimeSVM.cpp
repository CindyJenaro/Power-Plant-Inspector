/* As Client */
#include <opencv2/opencv.hpp>  
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <io.h>
#include <direct.h>
#include<winsock.h>
#pragma comment(lib, "WS2_32.lib") // 表示链接Ws2_32.lib这个库。Windows下的socket程序依赖 Winsock.dll或 ws2_32.dll。

using namespace std;
using namespace cv;
using namespace cv::ml;
#define endl '\n'

char resultfile[100];

#define SAVE_REPORTED

string path_prefix 
	= "C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\Integrated CV Test\\";
string model_pathname 
	= "D:\\[Don't_Delete!]Project1\\svm_auto.xml";
string reported_pathformat
	= path_prefix + "\\REPORTED\\Reported %d.png";

void init_wsa();
SOCKET init_recv_socket(int port);
bool recvfrom_socket(int port, SOCKET socket, char recvline[1024]);
void close_socket(int port, SOCKET socket);
void investigate(string croppedFilePath);
void getFiles(string path, vector<string>& files);

Ptr<SVM> svm;
char recvline4000[1024];
SOCKET recv_sock4000;
int globalcnt = 0;

int main()
{
	svm = SVM::create();
	FileStorage svm_fs(model_pathname, FileStorage::READ);
	if (svm_fs.isOpened())
	{
		svm = SVM::load(model_pathname.c_str());
		cout << "加载模型完成" << endl;
	}

	init_wsa();
	recv_sock4000 = init_recv_socket(4000);

	while(1)
	{
		if(recvfrom_socket(4000, recv_sock4000, recvline4000))
			investigate(string(recvline4000));
	}

	WSACleanup();

	return 0;
}

void init_wsa()
{
	WSADATA wsaData; 
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SOCKET init_recv_socket(int port)
{
	printf("creating socket of port %d...\n", port);
	SOCKET RecvSocket; 
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	timeval tv = {1000, 0}; 
	setsockopt(RecvSocket, SOL_SOCKET,
        SO_RCVTIMEO, (char*)&tv, sizeof(timeval));

	return RecvSocket;
}

bool recvfrom_socket(int port, SOCKET socket, char recvline[1024])
{
	sockaddr_in FromAddr; 
	FromAddr.sin_family = AF_INET;
	FromAddr.sin_port = htons(port); 
	FromAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int client_addr_length = sizeof(FromAddr);
	
	int recv_length = recvfrom(socket, recvline, 1024, 
			0, (SOCKADDR*)&FromAddr, &client_addr_length);

	if(recv_length > 0)
	{
		printf("recvline: %s\n", recvline);
		return true;
	}
	else
		return false;
}

void investigate(string croppedFilePath)
{
	vector<string> croppedFiles;
    getFiles(croppedFilePath, croppedFiles);
    cout << "读取选区文件完成" << endl;

    int number = croppedFiles.size();
    for(int i = 0; i < number; i++)
    {
    	Mat candidate = imread(croppedFiles[i]);
		Mat p = candidate.reshape(1, 1); // 变单通道单行
		p.convertTo(p, CV_32FC1);

		int response = (int)svm->predict(p);

		if (response == 1) 
		{
			++globalcnt;
#ifdef SAVE_REPORTED
			char reported_file[120];
			sprintf_s(reported_file, reported_pathformat.c_str(), globalcnt);
			imwrite(reported_file, candidate);
#endif
		}
    }

    cout << "报告水滴完成" << endl;
}

void getFiles(string path, vector<string>& files)
{
    intptr_t hFile = 0; 
    struct _finddata_t fileinfo;
    string p;
    int i = 30;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib &  _A_SUBDIR)) 
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFiles(p.assign(path).append("\\").append(fileinfo.name), files); // 递归进入子文件夹
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
 
        } while (_findnext(hFile, &fileinfo) == 0);
 
        _findclose(hFile); 
    }
}