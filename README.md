# Power-Plant-Inspector
清华大学技创辅82项目：电厂巡检机器小车研究数据集  

#### 关于数据

> 代码生成的过程数据和结果数据，由于文件太大无法上传至git，可移步清华云盘：  
<u>https://cloud.tsinghua.edu.cn/#group/4445/lib/76d50336-c98e-49a0-a3df-7415cc6f3f19</u>  
其中，每个压缩文件中RESULT文件夹存放的是最终结果（神经网络之前），其他文件夹存放过程数据，参见`Step 123 FullProcess.cpp`  

#### 关于前期视频处理

> `dark.avi`用`Step 123 FullProcess.cpp`处理最开始效果并不好，但后来发现是两帧之间差别太小，即两帧间隔太短导致的。  
使用`格式工厂`降低帧率至**12帧/秒**，效果明显有所提升。
