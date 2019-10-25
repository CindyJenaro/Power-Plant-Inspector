#include <stdio.h>  
#include <time.h>  
#include <opencv2/opencv.hpp>  
#include <opencv/cv.h>  
#include <iostream> 
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/ml/ml.hpp>  
#include <io.h>
 
using namespace std;
using namespace cv;
using namespace ml;
using namespace cv::ml;

int true_positive = 0;
int true_negative = 0;
int false_positive = 0;
int false_negative = 0;

void getFiles(string path, vector<string>& files);
 
int main()
{
	Ptr<SVM> svm = SVM::create();
 
	string modelpath = "D:\\Documents\\Datasets\\Crazy_Waterdrops\\svmtest\\svm_auto.xml";
	FileStorage svm_fs(modelpath, FileStorage::READ);
	if (svm_fs.isOpened())
	{
		svm = SVM::load(modelpath.c_str());
		cout << "loading model complete" << endl;
	}

	// positive first
	string positiveFilePath = "D:\\Documents\\Datasets\\Crazy_Waterdrops\\svmtest\\test1";
	vector<string> positiveFiles;
    getFiles(positiveFilePath, positiveFiles);
    cout << "positive-getFiles complete" << endl;

    int number = positiveFiles.size();
    for(int i = 0; i < number; i++)
    {
    	Mat inMat = imread(positiveFiles[i]);
		Mat p = inMat.reshape(1, 1); // 变单通道单行
		p.convertTo(p, CV_32FC1);
		int response = (int)svm->predict(p);
		if (response == 1) // predicted positive
			true_positive++;
		else if(response == 0)
			false_negative++;
    }
    cout << "positive-predictions complete" << endl;

    cout << true_positive << ' ' << true_negative << endl
		 << false_positive << ' ' << false_negative << endl;
	getchar();

    // negative then
    string negativeFilePath = "D:\\Documents\\Datasets\\Crazy_Waterdrops\\svmtest\\test0";
    vector<string> negativeFiles;
    getFiles(negativeFilePath, negativeFiles);
    cout << "negative-getFiles complete" << endl;

    number = negativeFiles.size();
    for(int i = 0; i < number; i++)
    {
    	Mat inMat = imread(negativeFiles[i]);
    	Mat p = inMat.reshape(1, 1); // 变单通道单行
    	p.convertTo(p, CV_32FC1);
    	int response = (int)svm->predict(p);
    	if (response == 0)
    		true_negative++;
    	else if(response == 1)
    		false_positive++;
    }
	cout << "negative-predictions complete" << endl;

	cout << true_positive << ' ' << true_negative << endl
		 << false_positive << ' ' << false_negative << endl;
	getchar();
	
	cout << "Accuracy: " 
		<< (float)(true_positive + true_negative)/(float)(true_positive + true_negative + false_positive + false_negative) 
		<< endl;
	cout << "Precision: "
		<< (float)true_positive/(float)(true_positive + false_positive) << endl;
	cout << "Recall: "
		<< (float)true_positive/(float)(true_positive + false_negative) << endl;
	cout << "F-score: "
		<< (float)2*true_positive/(float)(2*true_positive + false_positive + false_negative) << endl;
	getchar();
	return  0;
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


/*
————————————————
版权声明：本文为CSDN博主「北顾+」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/qq_40238526/article/details/92686721
已加入大量自己的修改。版权所有，水木清华，翠柏长生。
*/