# Power-Plant-Inspector
清华大学技创辅82项目：电厂巡检机器小车研究数据集

#### 关于数据
###### 代码生成的过程数据和结果数据
> 由于文件太大无法上传至git，可移步[**清华云盘**](https://cloud.tsinghua.edu.cn/#group/4445/lib/76d50336-c98e-49a0-a3df-7415cc6f3f19)  
其中，每个压缩文件中RESULT文件夹存放的是最终结果（神经网络之前），其他文件夹存放过程数据，参见`Step 123 FullProcess.cpp` 

###### 水滴自检数据
> 存放于清华云盘的[**Crazy_Waterdrops.rar**](https://cloud.tsinghua.edu.cn/lib/76d50336-c98e-49a0-a3df-7415cc6f3f19/file/Crazy_Waterdrops.rar)数据集上，其中文件夹：
- 1为有水滴训练数据
- 0为无水滴训练数据
- test1为有水滴测试数据
- test0为无水滴测试数据 
- backup1为有水滴备用数据
- backup0为无水滴备用数据
> 以上两个文件夹可以批量移图片至前面四个文件夹，从而调整训练、测试数据的多少和1/0比例（目前比例≈1：9）。  
> 以上六个文件夹图片大小均为31x31，格式为jpg。来自不同原始图片的训练、测试图片已通过随机修改文件名的方法尽量打乱顺序。
- svm.xml和svm_auto.xml是用代码中`#ifndef AUTO`和`#else`分别训练出来的两个SVM模型，waterdrops结果.txt分别记录其分类结果（很不理想）

###### 亮环境真实数据集
> 比自检数据集结果好得多得多的[**亮环境真实数据集.rar**](https://cloud.tsinghua.edu.cn/lib/76d50336-c98e-49a0-a3df-7415cc6f3f19/file/%E4%BA%AE%E7%8E%AF%E5%A2%83%E7%9C%9F%E5%AE%9E%E6%95%B0%E6%8D%AE%E9%9B%86.rar)  
阳性 : 阴性 = 147 : 591 ≈ 1 : 4（未经过筛选的真实比例）  
目前还未分出test数据集，下一步准备分出test数据集并进行数据集扩增  
  
#### 关于前期视频处理

> `dark.avi`用`Step 123 FullProcess.cpp`处理最开始效果并不好，但后来发现是两帧时间间隔太短，两帧画面差别太小，导致的水滴遗漏。   
使用格式工厂降低视频帧率至**12帧/秒**，效果明显有所提升。  
[**格式工厂无毒下载链接**](http://soft.onlinedown.net/soft/983615.htm)  

> **重要说明**：`Step 123 FullProcess.cpp`会crash。但（根据数十篇csdn&stackoverflow&etc博客）这不是代码的bug，而是opencv本身的bug，而且目前还没有很好的方法解决或绕开这个问题。crash的地点是`vector<vector<Point>> contours`的析构函数处，这个数据类型为opencv自身的接口`findContours()`所需要。现在可以保证整个程序运行期间不会crash，所有结果可以得到有效保存，但是退出程序时会crash一次。

#### 关于水滴识别

###### SVM版本
> 虽然之前的自检数据并不理想  
但当时早有预感视频数据的结果比自检数据还会好  
>  
以结果最好的Bright.avi为例  
阳性 : 阴性 = 147 : 591 ≈ 1 : 4  
>
>【在训练集上测试】  
svm_auto.xml 13,653KB  
TP: 147 TN: 590  
FP: 0 FN: 1  
Accuracy: 0.998645  
Precision: 1  
Recall: 0.993243  
F-score: 0.99661  
Total timecost: 2.659s  
注：auto_train()函数自身已保证有**交叉测试**  
>下一步计划：（双保险）确认好结果的出现不是过拟合；在Dark.avi等之上测试  
（备选）用SSD改进算法；在SVM之前做卷积池化；用模拟退火算法寻找最优卷积核  
同步进行YOLO试验


###### YOLO v3的python版本
> 基于一块单片机（Nano）的板子实现对图片的Inference；这里的Inference使用的是COCO的数据集，未进行新的数据的训练，更改detect.py至可以直接调用CSI摄像头（鱼眼摄像头），速度约为0.7s检测一张图片
>
> 在本地计算机的GPU上，Inference约为0.2s一张图片，相比Nano上未调用摄像头的速度（0.6s），推测在本地计算机使用GPU+调用摄像头得到的帧数并不能够提高到12帧左右
>
>考虑使用C++
