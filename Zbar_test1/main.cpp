#include <iostream>
#include <opencv2\opencv.hpp>
#include <zbar.h>

using namespace std;
using namespace cv;
using namespace zbar;

//Ѱ����������
static vector<cv::Point> FindBiggestContour(Mat src)
{
	int imax = 0; //����������������
	int imaxcontour = -1; //������������Ĵ�С
	std::vector<std::vector<cv::Point>>contours;
	findContours(src, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		int itmp = contourArea(contours[i]);//������õ���������С
		if (imaxcontour < itmp)
		{
			imax = i;
			imaxcontour = itmp;
		}
	}
	return contours[imax];
}

//��ȡ��ά��
void detect_decode_qrcode()
{
	Mat img = imread("pictures/test.jpg");
	if (img.empty())
	{
		cout << "reading images fails" << endl;
	}
	//�ҶȻ�
	Mat img_gray, img_bin;
	cvtColor(img, img_gray, COLOR_BGR2GRAY);
	threshold(img_gray, img_bin, 100, 255, THRESH_OTSU | THRESH_BINARY_INV);  //THRESH_BINARY_INV  ��ֵ��ȡ��						 
	vector<vector<Point>> contours, contours2; //������  �ҵ��������������ķ�ʽ����
	vector<Vec4i> hierarchy;
	findContours(img_bin, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i < contours.size(); i++)
	{											//�������еĴ�����									
		if (hierarchy[i][2] != -1 && ic == 0)  //��� ���������û�и����� hierarchy[i][2] != -1 ˵�����Ǵ�����������
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//���������0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//�ҵ���λ����Ϣ
		if (ic == 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	//��ά���м���Ӧ�����������������ģ��������3 ��ô����Ϊ�����ж�ά���
	if (contours2.size() != 3)
	{
		printf("finding 3 rects fails \n");
	}
	//�Ѷ�ά������������������һ���µĵ㼯
	Rect new_rect;
	vector<Point> all_points;
	for (int i = 0; i < contours2.size(); i++)
	{
		for (int j = 0; j < contours2[i].size(); j++)
			all_points.push_back(contours2[i][j]);
	}
	new_rect = boundingRect(all_points);  //���ݶ�ά�빹�ɵõ㼯���ҵ�һ����С��������е㼯 �ľ���
										  //  Rect rect(230, 5, 280, 290);//�������꣨x,y���;��εĳ�(x)��(y)
										  //  cv::rectangle(src, rect, Scalar(255, 0, 0),1, LINE_8,0);
	rectangle(img, new_rect, Scalar(0, 0, 255), 8, 0);
	Mat result_img = img_gray(new_rect);   //���ҵ��ľ��� �Ž��Ҷ�ͼ�У�����ͼƬ�Ϳ��Ը��ݾ����и������
	ImageScanner scanner;
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	int width = result_img.step;  //��Ϊ��һС�����ǽ�ȡ������
	int height = result_img.rows;
	uchar* raw = (uchar*)result_img.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar);
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "��ѯ��ά��ʧ�ܣ�����ͼƬ��" << endl;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		cout << "���ͣ�" << endl << symbol->get_type_name() << endl << endl;
		cout << "��ά�룺" << endl << symbol->get_data() << endl << endl;
	}
	imageZbar.set_data(NULL, 0);
	//imshow("mat",img);
	imshow("mat1", result_img);
}


//��ȡ������
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
	//1��//�ҶȻ�
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	//2����ֵ��
	//Canny(gray, canny, 100, 255);
	Mat bin;
	threshold(gray, bin, 0, 255, THRESH_OTSU | THRESH_BINARY_INV);
	//3����̬ѧ�˲�
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(12, 2));
	morphologyEx(bin, bin, MORPH_DILATE, element); //��̬ѧ�˲�  �ҵ������� ���
	//morphologyEx(canny, canny, MORPH_ERODE, element);
	//4��Ѱ����������
	Rect boundRect = boundingRect(Mat(FindBiggestContour(bin)));
	Mat result_img = gray(boundRect);
	imshow("mat2", result_img);

	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	int width = result_img.step;
	int height = result_img.rows;
	uchar* raw = (uchar*)result_img.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar); //ɨ������      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();
	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		cout << "��ѯ����ʧ�ܣ�����ͼƬ��" << endl;
	}
	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		cout << "���ͣ�" << endl << symbol->get_type_name() << endl << endl;
		cout << "���룺" << endl << symbol->get_data() << endl << endl;
	}
	imageZbar.set_data(NULL, 0);
}
//������
int main()
{
	detect_decode_qrcode();
	detect_decode_barcode();
	waitKey(0);
	return 0;
}