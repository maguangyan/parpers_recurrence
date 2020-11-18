#include <iostream>
#include <opencv2\opencv.hpp>
#include <zbar.h>

using namespace std;
using namespace cv;
using namespace zbar;

//寻找最大的轮廓
static vector<cv::Point> FindBiggestContour(Mat src)
{
	int imax = 0; //代表最大轮廓的序号
	int imaxcontour = -1; //代表最大轮廓的大小
	std::vector<std::vector<cv::Point>>contours;
	findContours(src, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		int itmp = contourArea(contours[i]);//这里采用的是轮廓大小
		if (imaxcontour < itmp)
		{
			imax = i;
			imaxcontour = itmp;
		}
	}
	return contours[imax];
}

//获取二维码
void detect_decode_qrcode()
{
	Mat img = imread("pictures/test.jpg");
	if (img.empty())
	{
		cout << "reading images fails" << endl;
	}
	//灰度化
	Mat img_gray, img_bin;
	cvtColor(img, img_gray, COLOR_BGR2GRAY);
	threshold(img_gray, img_bin, 100, 255, THRESH_OTSU | THRESH_BINARY_INV);  //THRESH_BINARY_INV  二值化取反						 
	vector<vector<Point>> contours, contours2; //找轮廓  找到的轮廓按照树的方式排列
	vector<Vec4i> hierarchy;
	findContours(img_bin, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i < contours.size(); i++)
	{											//遍历所有的大轮廓									
		if (hierarchy[i][2] != -1 && ic == 0)  //如果 这个大轮廓没有父轮廓 hierarchy[i][2] != -1 说明他是存在子轮廓的
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//最外面的清0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//找到定位点信息
		if (ic == 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	//二维码中间是应该有三个特征轮廓的，如果等于3 那么就认为它是有二维码的
	if (contours2.size() != 3)
	{
		printf("finding 3 rects fails \n");
	}
	//把二维码最外面的轮廓构造成一个新的点集
	Rect new_rect;
	vector<Point> all_points;
	for (int i = 0; i < contours2.size(); i++)
	{
		for (int j = 0; j < contours2[i].size(); j++)
			all_points.push_back(contours2[i][j]);
	}
	new_rect = boundingRect(all_points);  //根据二维码构成得点集，找到一个最小的外包所有点集 的矩形
										  //  Rect rect(230, 5, 280, 290);//左上坐标（x,y）和矩形的长(x)宽(y)
										  //  cv::rectangle(src, rect, Scalar(255, 0, 0),1, LINE_8,0);
	rectangle(img, new_rect, Scalar(0, 0, 255), 8, 0);
	Mat result_img = img_gray(new_rect);   //将找到的矩形 放进灰度图中，这样图片就可以根据矩形切割出来了
	ImageScanner scanner;
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	int width = result_img.step;  //因为这一小部分是截取出来的
	int height = result_img.rows;
	uchar* raw = (uchar*)result_img.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar);
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "查询二维码失败，请检查图片！" << endl;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		cout << "类型：" << endl << symbol->get_type_name() << endl << endl;
		cout << "二维码：" << endl << symbol->get_data() << endl << endl;
	}
	imageZbar.set_data(NULL, 0);
	//imshow("mat",img);
	imshow("mat1", result_img);
}


//获取条形码
void detect_decode_barcode()
{
	Mat src = imread("pictures/test.jpg");
	if (src.empty())
	{
		cout << "reading images fails" << endl;
	}
	Mat sobel;
	Mat canny;
	Mat canny_output;
	//1、//灰度化
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	//2、二值化
	//Canny(gray, canny, 100, 255);
	Mat bin;
	threshold(gray, bin, 0, 255, THRESH_OTSU | THRESH_BINARY_INV);
	//3、形态学滤波
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(12, 2));
	morphologyEx(bin, bin, MORPH_DILATE, element); //形态学滤波  找到条形码 大白
	//morphologyEx(canny, canny, MORPH_ERODE, element);
	//4、寻找最大的轮廓
	Rect boundRect = boundingRect(Mat(FindBiggestContour(bin)));
	Mat result_img = gray(boundRect);
	imshow("mat2", result_img);

	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	int width = result_img.step;
	int height = result_img.rows;
	uchar* raw = (uchar*)result_img.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar); //扫描条码      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "查询条码失败，请检查图片！" << endl;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		cout << "类型：" << endl << symbol->get_type_name() << endl << endl;
		cout << "条码：" << endl << symbol->get_data() << endl << endl;
	}
	imageZbar.set_data(NULL, 0);
}
//主函数
int main()
{
	detect_decode_qrcode();
	detect_decode_barcode();
	waitKey(0);
	return 0;
}