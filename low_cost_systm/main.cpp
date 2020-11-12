//#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cmath>

using namespace cv;
using namespace std;


float ratio = 0.049113; // 1个像素代表几个毫米

double getDistance(Point point1, Point point2)
{
	double distance;
	distance = powf((point1.x - point2.x), 2) + powf((point1.y - point2.y), 2);
	distance = sqrtf(distance);
	return distance;
}

/** @brief 中心特征法.
将迪卡儿坐标系中的一堆轮廓转换到极坐标中，将二维曲线转化为一维曲线，横坐标为角度θ，纵坐标为r.
@param cnts 轮廓，以vector<vector<Point>>形式表示
@param index 轮廓的索引值
@param cnt_img 返回cnt画出来的轮廓图
@parma output 返回轮廓的极坐标表示
 */
void centroidalProfile(vector<vector<Point>> cnts, int index, Mat&cnt_img, Mat& output)
{

	Mat polar_img;
	drawContours(cnt_img, cnts, index, Scalar(255, 255, 255), 1, LINE_4, Mat(), 0, Point(0, 0));
	Moments mm2 = moments(cnts[index]);
	double cx2 = mm2.m10 / mm2.m00;
	double cy2 = mm2.m01 / mm2.m00;
	circle(cnt_img, Point(cx2, cy2), 2, Scalar(0, 0, 255), -1, 8, 0);
	linearPolar(cnt_img, polar_img, Point(cx2, cy2), 900, INTER_LINEAR + WARP_FILL_OUTLIERS);
	transpose(polar_img, polar_img);
	int h = polar_img.rows;
	int w = polar_img.cols;
	output = Mat::zeros(h, w, polar_img.type());
	for (int i = 0; i < polar_img.rows; i++)
	{
		for (int j = 0; j < polar_img.cols; j++)
		{
			output.at<Vec3b>(i, j) = polar_img.at<Vec3b>(h - i - 1, j);
		}
	}
	putText(output, format("theta"), Point(w - 700, h - 50), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 255, 255), 2);
	putText(output, format("r"), Point(50, 200), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 255, 255), 2);
}

int main(int argc, char** argv)
{
	// 读取数据
	Mat src = imread("pictures/test6.jpg");

	if (src.empty())
	{
		cout << "could not open image..." << endl;
		return -1;
	}
	system("color 0A");	// 控制台黑底绿字
	cout << "图像的通道数为：" << src.channels() << endl;

	// 转灰度
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);

	// 直方图均衡化
	Mat equa_hist;
	equalizeHist(gray, equa_hist);
	// 模糊
	Mat gaussblur_img;
	GaussianBlur(equa_hist, gaussblur_img, Size(3, 3), 1);
	// 边缘提取
	Mat edge_img;
	Canny(gaussblur_img, edge_img, 130, 200, 3, false);

	//// 膨胀
	//Mat dilate_img;
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	//dilate(edge_img, dilate_img, kernel, Point(-1, -1), 1);

	// 轮廓分析
	vector<vector<Point>> contours, dst_contours;
	vector<Vec4i> hierachy, dst_hierachy;
	double max_area = 0;
	int max_area_index = 0;
	int count = 0;
	findContours(edge_img, contours, hierachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (int i = 0; i < contours.size(); i++)
	{
		double contour_area = contourArea(contours[i]);
		double contour_length = arcLength(contours[i], false);
		drawContours(src, contours, i, Scalar(0, 0, 255), 4, LINE_8, dst_hierachy, 0, Point(0, 0));
		if (contour_area > 15000 && contour_length > 500)
		{
			dst_contours.push_back(contours[i]);
			dst_hierachy.push_back(hierachy[i]);
			if (contour_area > max_area)
			{
				max_area = contour_area;
				max_area_index = i;
				count++;
			}
		}
	}
	// 绘制
	Mat dst_4 = Mat::zeros(src.size(), CV_8UC3);
	Mat dst_1 = Mat::zeros(src.size(), CV_8UC3);
	Mat dst_2 = Mat::zeros(src.size(), CV_8UC3);
	Mat dst_3 = Mat::zeros(src.size(), CV_8UC3);
	drawContours(dst_4, contours, max_area_index, Scalar(255, 255, 255), -1, LINE_4, dst_hierachy, 0, Point(0, 0));
	
	// 中心特征法分析轮廓
	
	Mat polar_img;
	centroidalProfile(contours, max_area_index, dst_3, polar_img);

	for (int i = 0; i < dst_contours.size(); i++)
	{
		Scalar color1 = Scalar(0, 255, 0);
		Scalar color2 = Scalar(255, 255, 255);

		if (i % 2 == 0)
		{
			drawContours(dst_1, dst_contours, i, color1, 4, LINE_8, dst_hierachy, 0, Point(0, 0));
			if ((i != count) && (i != (count - 1)))
			{
				drawContours(dst_2, dst_contours, i, color2, -1, LINE_8, dst_hierachy, 0, Point(0, 0));
			}
		}

	}
	// make mask
	Mat mask1, mask2, img;
	cvtColor(dst_2, mask1, COLOR_BGR2GRAY);
	cvtColor(dst_4, mask2, COLOR_BGR2GRAY);

	bitwise_not(mask1, mask1);
	bitwise_and(mask1, mask2, mask2);

	bitwise_and(edge_img, mask2, img);

	// surface detect
	vector<vector<Point>> cnts;
	vector<Vec4i> hier;
	findContours(img, cnts, hier, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	// ansys
	Mat result_show;
	src.copyTo(result_show);
	double cnt_area = 0;
	double cnt_length = 0;

	for (int i = 0; i < cnts.size(); i++)
	{
		cnt_area = contourArea(cnts[i]);
		drawContours(result_show, cnts, i, Scalar(255, 0, 0), 1); // 检测出来的所有轮廓 蓝色显示
		if (cnt_area > 0 && cnt_area < 100)
		{
			if (cnts[i].size() < 5) continue;


			drawContours(result_show, cnts, i, Scalar(0, 255, 0), 1); // 检测出来的疑似缺陷的轮廓 绿色显示
			RotatedRect rrt2 = fitEllipseDirect(cnts[i]);
			double radius2 = min(rrt2.size.width, rrt2.size.height) / 2.0;
			float k = rrt2.size.width / rrt2.size.height;
			bool iswant = k > 0.8 && k < 1.2;

			if (iswant)
			{
				circle(result_show, rrt2.center, radius2 + 10, Scalar(0, 0, 255), 4, 8, 0);	// 确定是缺陷的轮廓，红色圈起来
				Moments mm2 = moments(cnts[i]);
				double cx2 = mm2.m10 / mm2.m00;
				double cy2 = mm2.m01 / mm2.m00;
				//circle(result_show, Point(cx2, cy2), 1, Scalar(255, 0, 0), 1, 8, 0);
				//putText(result_show, format("%fpix", radius2 * 2), rrt2.center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 1, 8);
				putText(result_show, format("%fmm", radius2 * 2 * ratio), rrt2.center, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 1, 8);

			}
		}

	}
	waitKey(0);
	return 0;
}