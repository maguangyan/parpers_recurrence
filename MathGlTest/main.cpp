#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <algorithm>
#include <vector>

#include "mgl2/mgl.h"
using namespace std;
using namespace cv;


int sample(mglGraph* gr)
{
	mglData x(10000), y(10000), z(10000);  
	gr->Fill(x, "2*rnd-1");
	gr->Fill(y, "2*rnd-1"); 
	//gr->Fill(z, "exp(-6*(v^2+w^2))", x, y);
	gr->Fill(z, "exp(-2*(v^3+w^3))", x, y);

	mglData xx = gr->Hist(x, z), yy = gr->Hist(y, z);	
	xx.Norm(0, 1);
	yy.Norm(0, 1);
	gr->MultiPlot(3, 3, 3, 2, 2, "");   gr->SetRanges(-1, 1, -1, 1, 0, 1);
	gr->Box();  gr->Dots(x, y, z, "wyrRk");
	gr->MultiPlot(3, 3, 0, 2, 1, "");   gr->SetRanges(-1, 1, 0, 1);
	gr->Box();  gr->Bars(xx);
	gr->MultiPlot(3, 3, 5, 1, 2, "");   gr->SetRanges(0, 1, -1, 1);
	gr->Box();  gr->Barh(yy);
	gr->SubPlot(3, 3, 2);
	gr->Puts(mglPoint(0.5, 0.5), "Hist and\nMultiPlot\nsample", "a", -6);
	return 0;
}

Mat my_drawHist(Mat gray)
{
	// 双峰法 分通道显示 计算直方图
	int histSize = 256;	// 直方图bin的个数
	float range[] = { 0, 256 };	// 输入数据的维度 灰度值范围为0-255
	const float* histRanges = { range };
	Mat hist;
	calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, &histRanges, true, false);
	double hist_min;
	double hist_max;
	//int hist_min_index;
	//int hist_max_index;
	minMaxLoc(hist, &hist_min, &hist_max);
	// 归一化
	int hist_h = 1024;
	int hist_w = 1024;
	int bin_w = hist_w / histSize;
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0)); // 创建黑色背景
	normalize(hist, hist, hist_h, 0, NORM_MINMAX, -1, Mat());

	// 绘制直方图
	Scalar yellow = Scalar(102, 255, 255);
	Scalar red = Scalar(0, 0, 255);
	int bin_h = hist_h / 8;
	float bin_frequency = hist_max / hist_h;
	for (int i = 0; i < 7; i++)
	{
		line(histImage, Point(0, hist_h - bin_h * (i + 1)), Point(hist_w - 1, hist_h - bin_h * (i + 1)), yellow, 1, 8);
		int frequency = bin_frequency * (i + 1);
		putText(histImage, format("%d", frequency * bin_h), Point(0, hist_h - bin_h * (i + 1)), FONT_HERSHEY_SIMPLEX, 1, yellow, 1, LINE_4);
	}


	RNG rng(123);
	Point line_start;
	Point line_end;

	line_start = Point(0, hist_h - cvRound(hist.at<float>(0)));
	for (int i = 0; i < histSize; i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		rectangle(histImage, Point(i * bin_w, hist_h), Point((i + 1) * bin_w - 1, hist_h - cvRound(hist.at<float>(i))), color, -1);
		if ((i + 1) % 20 == 0)
		{
			putText(histImage, format("%d", i + 1), Point(Point(i * bin_w, hist_h)), FONT_HERSHEY_SIMPLEX, 1, yellow, 1, LINE_4);
		}
		if (hist.at<float>(i) == 0)
		{
			continue;
		}
		else
		{
			line_end = Point(i * bin_w, hist_h - cvRound(hist.at<float>(i)));
			line(histImage, line_start, line_end, Scalar(0, 0, 255), 1, LINE_4);

			// update
			line_start = line_end;

		}
	}
	return histImage;
}

int my_hist(mglGraph* gr, Mat gray)
{

	int histSize = 256;	// 直方图bin的个数
	float range[] = { 0, 256 };	// 输入数据的维度 灰度值范围为0-255
	const float* histRanges = { range };
	Mat hist;
	calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, &histRanges, true, false);
	
	mglData y;
	mglData* py = &y;
	py->Create(256);

	for (int i = 0; i < hist.rows; i++)
	{
		py->a[i] = hist.at<float>(i);
	}
	
	//y.Norm(mreal(0), mreal(1));
	gr->SetRanges(0, 255, 0, y.Maximal());
	gr->Axis(); // 坐标轴
	gr->Box(); 
	gr->Bars(y);
	return 0;
}

void smgl_3wave(mglGraph* gr)
{
	gr->SubPlot(1, 1, 0, "<_");
	gr->Title("Complex ODE sample");
	double t = 50;
	mglData ini;	ini.SetList(3, 1., 1e-3, 0.);
	mglDataC r(mglODEc("-b*f;a*conj(f);a*conj(b)-0.1*f", "abf", ini, 0.1, t));
	gr->SetRanges(0, t, 0, r.Maximal());
	gr->Plot(r.SubData(0), "b", "legend 'a'");
	gr->Plot(r.SubData(1), "g", "legend 'b'");
	gr->Plot(r.SubData(2), "r", "legend 'f'");
	gr->Axis();	gr->Box();	gr->Legend();
}

int main(int argc, char** argv) 
{
	Mat src, dst;
	src = imread("pictures/test.jpg", IMREAD_COLOR);//src 缩写为源文件 打开的文件名要加上格式 比如 jpg
	if (src.empty())
	{
		printf("could not load image....\n");
		return -1;//返回值为-1 表示异常
	}
	namedWindow("input", CV_WINDOW_AUTOSIZE);//创建一个窗口并命名
	imshow("input", src);//在指定窗口显示指定图片
	
	cvtColor(src, dst, COLOR_BGR2GRAY);

	Mat hist_img = my_drawHist(dst);
	imshow("hist_opencv", hist_img);
	//创建gr对象，指定图像大小为800x500,kind=0说明不使用OpenGL
	mglGraph gr1(0, 800, 500);
	mglGraph gr2(0, 800, 500);

	sample(&gr1);
	my_hist(&gr2, dst);
	//smgl_3wave(&gr1);

	//用OpenCV显示图片
	Mat pic(gr1.GetHeight(), gr1.GetWidth(), CV_8UC3);
	pic.data = const_cast<uchar*>(gr1.GetRGB());
	imshow("test", pic);

	Mat pic2(gr2.GetHeight(), gr2.GetWidth(), CV_8UC3);
	pic2.data = const_cast<uchar*>(gr2.GetRGB());
	imshow("hist_mathGL", pic2);

	////保存图片
	//std::cout << "write image as \"test.png\"." << std::endl;
	//gr.WritePNG("test.png");  // Don't forget to save the result!

	waitKey(0);
	return 0;
}