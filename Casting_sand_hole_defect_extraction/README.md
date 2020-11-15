### 铸件砂眼缺陷提取的图像处理算法

铸造技术（普刊） 2018年第七期

提出一种图像形态学重构后的差分算法。 Matlab 读取图像，中值滤波对图 像去噪减少干扰信息，再对去噪后的缺陷图像和合格图像分别进行增强和形态学处理；随后采用差分算法求出两幅图 像之间的差异信息，得出图像中灰度值最大的区域即为砂眼所在的位置；最后求出砂眼所占区域的面积、周长、圆形度， 累积砂眼特征库

#### 文章脉络

```c++
一. 图像预处理：
	1. 转灰度
	2. 图像去噪 中值滤波去椒盐噪声
	3. 均衡化
	4. 形态学操作 腐蚀，增大砂眼>>开操作,去除毛刺 // 注意选择合适的kernel，对于砂眼来说，圆盘disk比较合适
二. 砂眼提取：
    1. 砂眼定位 差分法，与标准图像进行比较，得到差分图
    2. 特征提取 对差分图使用canny算子进行边缘检测，进行分析轮廓周长、面积、圆形度等 // 选择合适的阈值
    
```

针对法兰盘，代码复现：

```c++
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cmath>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	// 读取数据
	Mat src = imread("pictures/src.jpg");
	Mat std_img = imread("pictures/std.jpg");

	if (src.empty())
	{
		cout << "could not open image..." << endl;
		return -1;
	}
	system("color 0A");	// 控制台黑底绿字
	cout << "图像的通道数为：" << src.channels() << endl;

	// 转灰度
	Mat gray, std_gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	cvtColor(std_img, std_gray, COLOR_BGR2GRAY);

	// 中值滤波
	Mat medianblur_img;
	medianBlur(gray, medianblur_img, 3);

	// 直方图均衡化
	Mat equa_hist;
	equalizeHist(gray, equa_hist);

	// 腐蚀
	Mat erode_img;
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3,3));
	erode(equa_hist, erode_img, kernel, Point(-1, -1), 1);
	// 开操作
	Mat open_img;
	morphologyEx(erode_img, open_img, MORPH_OPEN, kernel);

	// 砂眼定位 差分法
	Mat diff_img;
	absdiff(gray, std_gray, diff_img);


	// 边缘提取
	Mat edge_img;
	Canny(medianblur_img, edge_img, 130, 200, 3, false);
    // 分析轮廓。。。
    
	waitKey(0);
	return 0;
}
```

因为每次法兰盘的角度都不一样，导致采集的图片中法兰盘的位置都不固定，这不太适合使用差分法。

倒也是可以通过旋转矫正法兰盘位置，然后再和标准图片进行差分，但这样做无疑增加了算法的复杂度，误差更大。

目前还是使用掩膜的方式比较好。