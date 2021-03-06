/* As Server */
#include "MvCameraControl.h"
#include <iostream>
#include <windows.h> // Sleep()
#include <opencv2/opencv.hpp>
#include <winsock.h>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <direct.h>
#pragma comment(lib, "WS2_32.lib") 

using namespace std;
using namespace cv;
using namespace cv::ml;

#pragma comment(lib, "D:\\MVS\\Development\\Libraries\\win64\\MvCameraControl.lib")
#define MAX_BUF_SIZE (1920*1200*3)
#define endl '\n'

// #define TEXT_ERR
#define SAVE_THRES
#define SAVE_MORPH
#define SAVE_CANNY

const int sock_timeout = 83;
const int thres = 20;
const int op = MORPH_OPEN; 
const int elem = 0; 
const int size = 3; 
const int iter = 2; 
// 超参数

string path_prefix 
	= "C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\Integrated CV Test\\";
string stderr_pathname
	= path_prefix + "\\stderr.txt";
string orig_pathformat 
	= path_prefix + "\\ORIGINAL\\Original %d.png";
string thres_pathformat
	= path_prefix + "\\THRES\\Thres %d.png";
string morph_pathformat
	= path_prefix + "\\MORPH\\Morph %d.png";
string canny_pathformat
	= path_prefix + "\\CANNY\\Canny %d.png";
string cropped_pathformat
	= path_prefix + "\\CROPPED\\FRAME %d\\Cropped %d.png";
	// 固定路径参数

void* connect_device();
void disconnect_device(void*);
void start_realtimecv(void*);
void stop_realtimecv(void*);
void init_wsa();
SOCKET init_recv_socket(int port);
SOCKET init_send_socket(int port);
void recvfrom_socket(int port, SOCKET socket, char recvline[1024]);
void sendto_socket(int port, SOCKET socket, const char* message);
void close_socket(int port, SOCKET socket);
void create_paths();
int create_directory(string path);
void read_save_diff();
void canny_crop_images();
void report_waterdrops();

char recvline2000[1024];
char origfile[100];
char thresfile[100];
char morphfile[100];
char cannyfile[100];
SOCKET recv_sock2000, send_sock4000;
Mat previousframe, currentframe, difframe, cannyframe, cropped_raw, cropped;
vector<vector<Point>> contours;
vector<Vec4i> hierarchy;
clock_t start;

int main()
{
	create_paths();
	void* camera_handle = connect_device();

	init_wsa();
	recv_sock2000 = init_recv_socket(2000); // 从虚拟机那边跨机通信，接收“开始”“结束”信号
	send_sock4000 = init_send_socket(4000); // 向SVM那边发送“开始”信号
	
	while(1)
	{
		recvfrom_socket(2000, recv_sock2000, recvline2000);

		if(strcmp(recvline2000, "exit") == 0)
		{
			close_socket(2000, recv_sock2000);
			break;
		}
		else if(strcmp(recvline2000, "startgrab") == 0)
			start_realtimecv(camera_handle);
		else if(strcmp(recvline2000, "stopgrab") == 0)
			stop_realtimecv(camera_handle);
	}

	WSACleanup();

	disconnect_device(camera_handle);

	getchar();

	return 0;
}

void create_paths()
{
	create_directory(path_prefix + "\\ORIGINAL\\");
	create_directory(path_prefix + "\\THRES\\");
	create_directory(path_prefix + "\\MORPH\\");
	create_directory(path_prefix + "\\CANNY\\");
	create_directory(path_prefix + "\\CROPPED\\");
	create_directory(path_prefix + "\\REPORTED\\");
}

int create_directory(string path)
{
	int len = path.length();
	char tmpDirPath[256] = { 0 };
	for (int i = 0; i < len; i++)
	{
		tmpDirPath[i] = path[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (_access(tmpDirPath, 0) == -1)
			{
				int ret = _mkdir(tmpDirPath);
				if (ret == -1) return ret;
			}
		}
	}
	return 0;
}

//----------------摄像头相关自封脚本--------------------//

void* connect_device()
{
	// 枚举子网内指定的传输协议对应的所有设备
	unsigned int layer_type = MV_GIGE_DEVICE;
	MV_CC_DEVICE_INFO_LIST devlist = { 0 };
	MV_CC_EnumDevices(layer_type, &devlist);
 
	// 选择查找到的第一台在线设备，创建设备句柄
	int device_index = 0;
 
	MV_CC_DEVICE_INFO dev_info = { 0 };
	memcpy(&dev_info, devlist.pDeviceInfo[device_index], sizeof(MV_CC_DEVICE_INFO));
	void* camera_handle = NULL;
	MV_CC_CreateHandle(&camera_handle, &dev_info);
 
	// 连接设备
	unsigned int access_mode = MV_ACCESS_Exclusive;
	unsigned short switchover_key = 0;
	MV_CC_OpenDevice(camera_handle, access_mode, switchover_key);

	return camera_handle;
}

void disconnect_device(void* camera_handle)
{
	MV_CC_CloseDevice(camera_handle);
	MV_CC_DestroyHandle(camera_handle);
}

//----------------UDP通讯相关自封脚本--------------------//

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
	
	sockaddr_in AnyAddr; 
	AnyAddr.sin_family = AF_INET;
	AnyAddr.sin_port = htons(port); 
	AnyAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

	printf("binding socket of port %d with address...\n", port);
	bind(RecvSocket, (SOCKADDR*)&AnyAddr, sizeof(AnyAddr));

	timeval tv = {sock_timeout, 0}; 
	setsockopt(RecvSocket, SOL_SOCKET,
        SO_RCVTIMEO, (char*)&tv, sizeof(timeval));

	return RecvSocket;
}

SOCKET init_send_socket(int port)
{
	SOCKET SendSocket; 
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	sockaddr_in AnyAddr; 
	AnyAddr.sin_family = AF_INET;
	AnyAddr.sin_port = htons(port); 
	AnyAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

	printf("binding socket of port %d with address...\n", port);
	bind(SendSocket, (SOCKADDR*)&AnyAddr, sizeof(AnyAddr));

	return SendSocket;
}

void recvfrom_socket(int port, SOCKET socket, char recvline[1024])
{
	sockaddr_in FromAddr; // 注意这并没有初始化
	int from_addr_length = sizeof(FromAddr);
	
	int recv_length = recvfrom(socket, recvline, 1024, 
			0, (SOCKADDR*)&FromAddr, &from_addr_length);
	if(recv_length > 0)
		printf("recvline: %s\n", recvline);
}

void sendto_socket(int port, SOCKET socket, const char* message)
{
	sockaddr_in ToAddr; 
	ToAddr.sin_family = AF_INET;
	ToAddr.sin_port = htons(port); 
	ToAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  

	static char sendline[1024];

	strcpy(sendline, message);

	int send_length = sendto(socket, sendline, 1024, 
		0, (SOCKADDR *)&ToAddr, sizeof(ToAddr));
}

void close_socket(int port, SOCKET socket)
{
	printf("finished receiving, closing socket of port %d..\n", port);
	closesocket(socket);
}

//---------------CV相关自封脚本-----------------//

void start_realtimecv(void* camera_handle)
{
	cerr << "Started grabbing..." << endl;
	MV_CC_StartGrabbing(camera_handle);

	int bufsize = MAX_BUF_SIZE;
	unsigned char*  framebuf = NULL;
	framebuf = (unsigned char*)malloc(bufsize);
 
	MV_FRAME_OUT_INFO_EX info;
	memset(&info, 0, sizeof(MV_FRAME_OUT_INFO_EX));

	int origframe_idx = 0;
	while (1)
	{
		start = clock();

		recvfrom_socket(2000, recv_sock2000, recvline2000);
		if(strcmp(recvline2000, "stopgrab") == 0)
		{
			stop_realtimecv(camera_handle);
			break;
		}


		// 第一帧		
		MV_CC_GetImageForBGR(camera_handle, framebuf, bufsize, &info, 1000);
		int width = info.nWidth;
		int height = info.nHeight;
		previousframe = Mat(height, width, CV_8UC3, framebuf).clone();
#ifdef SAVE_ORIG
		sprintf(origfile, "D:\\[Don't_Delete!]Project1\\Integrated CV Test\\ORIGINAL\\Original %d.jpg", origframe_idx);
		imwrite(origfile, previousframe);
#endif
		// Sleep(83); // 1000/12 = 83。保持每秒12帧的帧率。 
		++origframe_idx;

		// 第二帧		
		MV_CC_GetImageForBGR(camera_handle, framebuf, bufsize, &info, 1000);
		width = info.nWidth;
		height = info.nHeight;
		currentframe = Mat(height, width, CV_8UC3, framebuf).clone();
#ifdef SAVE_ORIG
		sprintf(origfile, "D:\\[Don't_Delete!]Project1\\Integrated CV Test\\ORIGINAL\\Original %d.jpg", origframe_idx);
		imwrite(origfile, currentframe);
#endif
		// Sleep(83); // 1000/12 = 83。保持每秒12帧的帧率。
		++origframe_idx;


		// 两帧差处理
		cvtColor(previousframe, previousframe, CV_BGR2GRAY);
		cvtColor(currentframe, currentframe, CV_BGR2GRAY);
		absdiff(currentframe, previousframe, difframe); // 作差求绝对值

		threshold(difframe, difframe, 15, 255.0, CV_THRESH_BINARY);
#ifdef SAVE_THRES
		sprintf_s(thresfile, thres_pathformat.c_str(), origframe_idx/2);
		imwrite(thresfile, difframe);
#endif


		// 开运算：先腐蚀，再膨胀，可清除一些小东西(亮的)，放大局部低亮度的区域
		Mat element = getStructuringElement(elem, Size(size, size));
		morphologyEx(difframe, difframe, op, element, Point(-1, -1), iter); 
#ifdef SAVE_MORPH
		sprintf_s(morphfile, morph_pathformat.c_str(), origframe_idx/2);
		imwrite(morphfile, difframe);
#endif


		// CANNY运算
		GaussianBlur(difframe, cannyframe, Size(3,3), 0);
		Canny(cannyframe, cannyframe, 150, 250);
#ifdef SAVE_CANNY
		sprintf_s(cannyfile, canny_pathformat.c_str(), origframe_idx/2);
		imwrite(cannyfile, cannyframe);
#endif

		// 框选可疑区域
		findContours(cannyframe, contours, hierarchy, RETR_EXTERNAL, 
					CHAIN_APPROX_NONE, Point()); 

		create_directory(path_prefix + "\\CROPPED\\FRAME " + to_string(origframe_idx - 2) + "\\");
		create_directory(path_prefix + "\\CROPPED\\FRAME " + to_string(origframe_idx - 1) + "\\");
		for(int contour_idx = 0; contour_idx < contours.size(); contour_idx++)
		{ 
			char croppedfile[120];
			static int cropped_idx = 0;
			cropped_idx++;
			// 静态局部变量的生存期虽然为整个源程序，但是其作用域仍与自动变量相同。

			int minx = 32767, miny = 32767, maxx = 0, maxy = 0;

			for(int point_idx = 0; point_idx < contours[contour_idx].size(); point_idx++)
			{
				if(contours[contour_idx][point_idx].x < minx) minx = contours[contour_idx][point_idx].x;
				if(contours[contour_idx][point_idx].y < miny) miny = contours[contour_idx][point_idx].y;
				if(contours[contour_idx][point_idx].x > maxx) maxx = contours[contour_idx][point_idx].x;
				if(contours[contour_idx][point_idx].y > maxy) maxy = contours[contour_idx][point_idx].y;
			} // endfor: point_idx

			// EXPAND
			minx = (minx - 10 < 0)? 0 : (minx - 10);
			maxx = (maxx + 10 > 1919)? 1919 : (maxx + 10);
			miny = (miny - 10 < 0)? 0 : (miny - 10);
			maxy = (maxy + 10 > 1199)? 1199 : (maxy + 10);
			
			// 前一帧
			cropped_raw = previousframe(Rect(minx, miny, maxx - minx, maxy - miny));
			resize(cropped_raw, cropped, Size(40, 40)); // resize函数不支持in-place
			sprintf_s(croppedfile, cropped_pathformat.c_str(), origframe_idx - 2, cropped_idx);
			imwrite(croppedfile, cropped); 
			
			// 后一帧
			cropped_raw = currentframe(Rect(minx, miny, maxx - minx, maxy - miny));
			resize(cropped_raw, cropped, Size(40, 40)); // resize函数不支持in-place
			sprintf_s(croppedfile, cropped_pathformat.c_str(), origframe_idx - 1, cropped_idx);
			imwrite(croppedfile, cropped); 
		} // endfor: contour_idx

		sendto_socket(4000, send_sock4000, 
			(path_prefix + "\\CROPPED\\FRAME " + to_string(origframe_idx - 2) + "\\").c_str());
		sendto_socket(4000, send_sock4000, 
			(path_prefix + "\\CROPPED\\FRAME " + to_string(origframe_idx - 1) + "\\").c_str());

	} // end while (frame capturing)
}

void stop_realtimecv(void* camera_handle)
{
	cerr << "Stopped grabbing." << endl;
	MV_CC_StopGrabbing(camera_handle);
}