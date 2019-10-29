#include <opencv2/opencv.hpp>  
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <io.h>

using namespace std;
using namespace cv;
using namespace cv::ml;
#define endl '\n'

char origfile[100];
char thresfile[100];
char morphfile[100];
char cannyfile[100];
char resultfile[100];

// #define TEXT_ERR
// #define SAVE_ORIG
// #define SAVE_THRES
// #define SAVE_MORPH
// #define SAVE_CANNY
// #define SAVE_CROPPED
#define SAVE_REPORTED

const int thres = 20;
const int op = MORPH_OPEN; 
const int elem = 0; 
const int size = 3; 
const int iter = 2; 
// 超参数

string video_name = "Bright"; // 可调，用哪个视频

string video_pathname 
	= "C:/Users/Cindy/Desktop/技创辅的团队/RAW DATA/" + video_name + ".avi";
string stderr_pathname
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/stderr.txt";
string orig_pathformat 
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/ORIGINAL/Original %d.png";
string thres_pathformat
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/THRES/Thres %d.png";
string morph_pathformat
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/MORPH/Morph %d.png";
string canny_pathformat
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/CANNY/Canny %d.png";
string cropped_pathformat
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/CROPPED/Cropped %d.png";
string reported_pathformat
	= "C:/Users/Cindy/Desktop/[Don't_Delete!]Project1/SVM+CNN Related/" + video_name + "/REPORTED/Reported %d.png";
string model_pathname
	= "C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\SVM+CNN Related\\Bright\\CROPPED\\svm_linear.xml";
	// 固定路径参数

vector<vector<Point>> contours;
vector<Vec4i> hierarchy;

void read_save_frames();
void canny_crop_images();
void report_waterdrops();
void getFiles(string path, vector<string>& files);

clock_t start;

int main()
{
#ifdef TEXT_ERR
	freopen("C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\SVM+CNN Related\\Bright\\nstderr.txt",
		"a", stderr);
	cerr << endl << endl;
#endif
	start = clock();
	read_save_frames();
	canny_crop_images();
	report_waterdrops();

	cout << "弄完了" << endl;

	string permission;
	while(permission != "nowcrash")
		cin >> permission;
	
	return 0;
}

void read_save_frames()
{
	cerr << "读取视频开始" << endl;
	VideoCapture capture(video_pathname);  
	Mat frame;
	Mat currentframe, previousframe, difframe;


	//读取一帧处理  
	while (true)
	{
		static int cnt = 0;
		if (!capture.isOpened())
		{
			cerr << "read video failure" << endl;
			return;
		}


		capture >> frame;
		if(!frame.data) break;
		previousframe = frame.clone(); //第一帧
		++cnt;
#ifdef SAVE_ORIG
		sprintf_s(origfile, orig_pathformat.c_str(), cnt); // 保存第一帧
 		imwrite(origfile, frame);
#endif


 		capture >> frame;
		if(!frame.data) break;
		currentframe = frame.clone();  //第二帧
		++cnt;
#ifdef SAVE_ORIG
		sprintf_s(origfile, orig_pathformat.c_str(), cnt); // 保存第二帧
 		imwrite(origfile, frame);
#endif


 		cvtColor(previousframe, previousframe, CV_BGR2GRAY);
		cvtColor(currentframe, currentframe, CV_BGR2GRAY);
		absdiff(currentframe, previousframe, difframe); // 作差求绝对值

		threshold(difframe, difframe, 15, 255.0, CV_THRESH_BINARY);
#ifdef SAVE_THRES
		sprintf_s(thresfile, thres_pathformat.c_str(), cnt/2);
		imwrite(thresfile, difframe);
#endif


		Mat element = getStructuringElement(elem, Size(size, size));
		morphologyEx(difframe, difframe, op, element, Point(-1, -1), iter); 
		// 开运算：先腐蚀，再膨胀，可清除一些小东西(亮的)，放大局部低亮度的区域
#ifdef SAVE_MORPH
		sprintf_s(morphfile, morph_pathformat.c_str(), cnt/2);
		imwrite(morphfile, difframe);
#endif
	} // end while (reading videocapture)

	clock_t completereading = clock();
	cerr << "读取视频完成，到此用时" << (double)(completereading - start)/CLOCKS_PER_SEC << "秒" << endl;
}


void canny_crop_images()
{
	cerr << "轮廓识别开始" << endl;
	for(int frame_idx = 1; ; frame_idx++)
	{
		sprintf_s(morphfile, morph_pathformat.c_str(), frame_idx);
		Mat imageSource = 
			imread(morphfile, 0); // 0使之灰度返回
		if(!imageSource.data) break;
		

		Mat image;
		GaussianBlur(imageSource, image, Size(3,3), 0);
		Canny(image, image, 150, 250);
#ifdef SAVE_CANNY
		sprintf_s(cannyfile, canny_pathformat.c_str(), frame_idx);
		imwrite(cannyfile, image);
#endif


#ifdef SAVE_CROPPED
		findContours(image, contours, hierarchy, RETR_EXTERNAL, 
					CHAIN_APPROX_NONE, Point()); 

		for(int origframe_idx = 2*frame_idx - 1; origframe_idx <= 2*frame_idx; origframe_idx++) // 对前后两帧依次操作
		{
			Mat orig;
			char origfile[120];

			sprintf_s(origfile, orig_pathformat.c_str(), origframe_idx);
			orig = imread(origfile);
			for(int contour_idx = 0; contour_idx < contours.size(); contour_idx++)
			{ 
				char croppedfile[120];
				static int cropped_idx = 0;
				cropped_idx++;
				// 静态局部变量的生存期虽然为整个源程序，但是其作用域仍与自动变量相同，即只能在定义该变量的函数内使用该变量。
				// 退出该函数后，尽管该变量还继续存在，但不能使用它。
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
				
				Mat cropped_raw = orig(Rect(minx, miny, maxx - minx, maxy - miny));
				Mat cropped;
				resize(cropped_raw, cropped, Size(40, 40)); // resize函数不支持in-place
				sprintf_s(croppedfile, cropped_pathformat.c_str(), cropped_idx);
				/* 原型: Rect(int_x, int_y, int_width, int_height); int_x和int_y代表左上角点的坐标 
					对比rectangle(在图片上画矩形框): void rectangle(..., Point pt1, Point pt2, ...) */
				imwrite(croppedfile, cropped); 
				cerr << "contour" << contour_idx + 1 << "/" << contours.size() << "complete\n"; 

			} // endfor: contour_idx
		} // endfor: origframe_idx
#endif
	} // endfor: frame_idx
	clock_t completecropping = clock();
	cerr << "切图完成，到此用时" << (double)(completecropping - start)/CLOCKS_PER_SEC << "秒" << endl;
}

void report_waterdrops()
{
#ifdef SAVE_REPORTED
	Ptr<SVM> svm = SVM::create();

	FileStorage svm_fs(model_pathname, FileStorage::READ);
	if (svm_fs.isOpened())
	{
		svm = SVM::load(model_pathname.c_str());
		cout << "加载模型完成" << endl;
	}

	string croppedFilePath = "C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\SVM+CNN Related\\" 
		+ video_name + "\\CROPPED\\test1";
	vector<string> croppedFiles;
    getFiles(croppedFilePath, croppedFiles);
    cout << "读取选区文件完成" << endl;

    int number = croppedFiles.size();
    for(int i = 0; i < number; i++)
    {
    	static int cnt = 0;
    	Mat candidate = imread(croppedFiles[i]);
		Mat p = candidate.reshape(1, 1); // 变单通道单行
		p.convertTo(p, CV_32FC1);
		int response = (int)svm->predict(p);
		if (response == 1) 
		{
			++cnt;
			char reported_file[120];
			sprintf_s(reported_file, reported_pathformat, cnt);
			imwrite(reported_file, candidate);
		}
    }

    clock_t completereporting = clock();
    cout << "报告水滴完成，到此用时" << (double)(completereporting - start)/CLOCKS_PER_SEC << "秒" << endl;
#endif
}

void getFiles(string path, vector<string>& files) // 把路径下所有文件的【路径+文件名】存在这个vector里
{
    intptr_t   hFile = 0; // pointer to handle(number) of file
    struct _finddata_t fileinfo;
    string p;
    int i = 30;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
 
        do
        {
            if ((fileinfo.attrib &  _A_SUBDIR)) // unsigned attrib: 文件属性
                // _A_ARCH（存档）、_A_HIDDEN（隐藏）、_A_NORMAL（正常）、_A_RDONLY（只读）、_A_SUBDIR（文件夹）、
                // _A_SYSTEM（系统），位组合。“与”表示只看这一位的属性。
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