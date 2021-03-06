#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <highgui.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <io.h> // 查找文件相关函数: _finddata_t _findfirst, _findnext, _findclose

using namespace std;
using namespace cv;
#define endl '\n'

void getFiles(string path, vector<string>& files);
int cap(int capped, int min, int max);
vector<string> files;
char enhanced_file[120];

int main()
{
	clock_t start, complete;
	start = clock();
	getFiles("C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\SVM+CNN Related\\Bright\\CROPPED\\0", files);
	int cnt = 0;
	Mat cropped, enhanced;
	for(int pic = 0; pic < files.size(); pic++)
	{
		cropped = imread(files[pic]);
		enhanced = cropped.clone();
		for(int brightness_bias = -3; brightness_bias <= 3; brightness_bias++)
		{
			for (int row = 0; row < cropped.rows; row++) 
			{
				for (int col = 0; col < cropped.cols; col++) 
				{
					for (int chn = 0; chn < 3; chn++) 
					{
						enhanced.at<Vec3b>(row, col)[chn] 
						= cap((cropped.at<Vec3b>(row, col)[chn] + brightness_bias), 0, 255);
					}
				}
			} // endfor: row
			++cnt;
			sprintf(enhanced_file, 
				"C:\\Users\\Cindy\\Desktop\\[Don't_Delete!]Project1\\SVM+CNN Related\\Bright\\CROPPED\\Enhanced\\0\\Enhanced %d.png", cnt);
			/* 重要！！！ 不要把生成文件搁在原文件的路径（包括子文件夹）下，否则无限循环不受控制！！！*/
			imwrite(enhanced_file, enhanced);
		} // endfor: brightness_bias
	} // endfor: pic
	
	complete = clock();
	cerr << "Total timecost: " << (double)(complete - start)/CLOCKS_PER_SEC << "s" << endl;
	getchar();
	return 0;
}


void getFiles(string path, vector<string>& files) // 把路径下所有文件的【路径+文件名】存在这个vector里
{
    intptr_t   hFile = 0; // pointer to handle of file
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

int cap(int capped, int min, int max)
{
	if(capped < min) return min;
	if(capped > max) return max;
	return capped;
}