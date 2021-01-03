#ifndef HEADERS
#define HEADERS

#include <cmath>
#include <string>
#include <fstream>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;
using namespace ml;

class Sex
{
public:
	string male = "��";
	string female = "Ů";
};

Mat getResizeGrayImage(const Mat &in);                             // ��ȡ�Ҷ�ͼ,����ͼ��С
void posDetect(const Mat &inputImage, vector<RotatedRect> &rects); // ���֤���붨λ�����Ӻ��߿�
bool isEligible(const RotatedRect &candidate);                     // �жϾ����Ƿ����Ҫ��
Mat Cut_Area(const Mat &inputImage, RotatedRect &rects_optimal);   // �ü����֤����
void splitCharacter(Mat &inputImage, vector<Mat> &dst_mat);        // �и������ַ�

void getAnnXML();                                   // ��ȡѵ������ͱ�ǩ���󣬲������xml�ļ�
void calcGradientFeat(const Mat &imgSrc, Mat &out); // ��������������
float sumMatValue(const Mat &image);                // ����Ҷ�ֵ��
Mat projectHistogram(const Mat &img, int t);        // ����ֱ��ͼ

void trainAnn(Ptr<ANN_MLP> &ann, int nlayers, int numCharacters);                       // ѵ��������
string classifyCharacter(Ptr<ANN_MLP> &ann, Ptr<ANN_MLP> &annX, vector<Mat> &char_Mat); // ���������ַ�
void recognitionRate(const string &inputString, string &char_result, int numb);         // ͳ�Ʋ���ӡʶ����
string identifyIdCard(const Mat &inputImage);                                           // ���֤ʶ��

#endif