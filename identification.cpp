#include "headers.h"

// ��ȡ�Ҷ�ͼ,����ͼ��С
Mat getResizeGrayImage(const Mat &in) {
  Mat resize_Gray, tempImage;
  cvtColor(in, tempImage, COLOR_BGR2GRAY);           // RGBת�Ҷ�
  bilateralFilter(tempImage, resize_Gray, -1, 5, 5); // ˫���˲�������ͼ���Ե(����)

  // ����ͼƬ��С
  if (resize_Gray.rows > 700 || resize_Gray.cols > 600) {
    Mat resizeR(450, 600, CV_8UC1);
    cv::resize(resize_Gray, resizeR, resizeR.size());
    return resizeR;
  } else
    return resize_Gray;
}

// ���֤���붨λ�����Ӻ��߿�
void posDetect(const Mat &intputImage, vector<RotatedRect> &rects) {
  Mat ImageBinary, tempImage;
  threshold(intputImage, ImageBinary, 0, 255, THRESH_OTSU); // ��ֵ��
  bitwise_not(ImageBinary, tempImage);                      // ��ת
  ImageBinary = tempImage;

  Mat element = getStructuringElement(MORPH_RECT, Size(15, 3)); // ���νṹԪ�أ�15*3�ӽ�����ȣ����֤������
  morphologyEx(ImageBinary, tempImage, MORPH_CLOSE, element);   // ������
  ImageBinary = tempImage;

  vector<vector<Point>> contours;                                        // ��������
  findContours(ImageBinary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); // ֻ�������������������߽������������������㵽contours������

  // Ѱ�����֤�����ڵ�����
  vector<vector<Point>>::iterator itc = contours.begin();
  while (itc != contours.end()) // ����ÿһ������
  {
    RotatedRect mr = minAreaRect(Mat(*itc)); // ��С�н��������
    if (isEligible(mr))                      // ������Ҫ��
    {
      rects.push_back(mr);
      ++itc;
    } else {
      itc = contours.erase(itc);
    }
  }

  Mat outputImage = intputImage.clone();
  Point2f vertices[4]; // ���ε�4�����㣬��������Ϊ�������������floatֵ
  if(rects.size() != 0)
    rects[0].points(vertices);
  for (int i = 0; i < 4; i++)
    line(outputImage, vertices[i], vertices[(i + 1) % 4], Scalar(0, 0, 0)); // ����ɫ�������ڶ����������ֱ�����߶ε���㡢�յ�
}

// �жϾ����Ƿ����Ҫ��
bool isEligible(const RotatedRect &candidate) {
  const double error = 0.3;         // �����
  const double aspect = 4.5 / 0.25; // ����ȣ����֤������

  double min = 10 * aspect * 10;         // ��С�������
  double max = 50 * aspect * 50;         // ����������
  double rmin = aspect - aspect * error; // �����������С�����
  double rmax = aspect + aspect * error; // �����������󳤿��

  double area = candidate.size.width * candidate.size.height;              // ��ǰ�������
  double r = (double)candidate.size.width / (double)candidate.size.height; // ��ǰ���򳤿��
  if (r < 1)
    r = 1 / r; // ��֤���νϳ��ı�Ϊ�����϶̵ı�Ϊ��

  if ((area >= min && area <= max) && (r >= rmin && r <= rmax)) // ���ڹ涨�������Χ�ͳ���ȷ�Χ��
    return true;
  else
    return false;
}

// �ü����֤����
Mat Cut_Area(const Mat &inputImage, RotatedRect &rects_optimal) {
  Mat output;
  float angle, r;

  angle = rects_optimal.angle;                                            // �Ƕ�
  r = (float)rects_optimal.size.width / (float)rects_optimal.size.height; // ��߱�
  if (r < 1)
    angle += 90; // ��ת90��,��֤�ϳ���һ��Ϊˮƽ״̬���϶̵�һ��Ϊ��ֱ״̬

  Mat rotmat = getRotationMatrix2D(rects_optimal.center, angle, 1); // ��ȡ�������ĵ���תangle�ǵı任����
  Mat img_rotated;
  // ��תͼ�񣬲����ֱ�Ϊ����תͼ����ת��ͼ�񡢱任������ת��ͼ���С��˫������ֵ�㷨
  warpAffine(inputImage, img_rotated, rotmat, inputImage.size(), INTER_CUBIC);

  Size rect_size = rects_optimal.size; // �����֤�ž��ο��С��ͬ�ľ��ο�
  if (r < 1)
    std::swap(rect_size.width, rect_size.height); // ��֤���νϳ��ı�Ϊ�����϶̵ı�Ϊ��
  Mat img_crop;
  // �ü�ͼ�񣬲����ֱ�Ϊ���ü�ͼ�񡢱��ü������С�����ü��������ġ��ü���ͼ��
  getRectSubPix(img_rotated, rect_size, rects_optimal.center, img_crop);

  // �ù���ֱ��ͼ�����ü���ͼ���������ͳһ�����������ѵ����ʶ���Ч��
  Mat resultResized;
  resultResized.create(20, 300, CV_8UC1);
  resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);

  output = resultResized.clone();
  return output;
}

// �и������ַ�
void splitCharacter(Mat &inputImage, vector<Mat> &dst_mat) {
  Mat img_threshold;
  threshold(inputImage, img_threshold, 0, 255, THRESH_OTSU); // ��ֵ��

  vector<bool> flag; // ��¼ÿһ�������������������ֵ�����
  int i, j;

  for (j = 0; j < img_threshold.cols; ++j) {
    flag.push_back(false);
    for (int i = 0; i < img_threshold.rows; ++i) {
      if (img_threshold.at<uchar>(i, j) == 0) // �׵׺��֣���Ϊ0
      {
        flag[j] = true;
        break;
      }
    }
  }

  int x[2][18] = {0};
  int count = 0;

  // ���֤��ǰ17������
  for (i = 0, j = 0; j < 17 && i + 2 < flag.size(); ++i) {
    if (flag[i] == true) {
      count++;

      // �������У���flag��Ϊfalse˵�����������ֵı�Ե
      if (flag[i + 1] == false && flag[i + 2] == false) {
        x[0][j] = i - (count - 1); // ���ֿ�ʼ������
        x[1][j] = count;           // ����ռ�е�����

        j++;
        count = 0;
      }
    }
  }

  // ��ü�����ͼ�������У�flag��Ϊtrue�������һ���������⴦��
  j = 17;
  for (; i < img_threshold.cols; ++i) {
    if (flag[i] == true)
      count++;

    if (i == img_threshold.cols - 1) {
      x[0][j] = i - (count - 1); // ���ֿ�ʼ������
      x[1][j] = count;           // ����ռ�е�����
    }
  }

  // ���д��
  Mat outputImage = 255 - inputImage; // �Ҷ�ͼ��ת����ѵ����������һ�£��ڵװ��֣�

  for (i = 0; i <= 17; i++) {
    dst_mat.push_back(Mat(outputImage, Rect(x[0][i], 0, x[1][i], outputImage.rows)));
  }
}



// ���������ַ�
string classifyCharacter(Ptr<ANN_MLP> &ann, Ptr<ANN_MLP> &annX, vector<Mat> &char_Mat) {
  string idResult = "000000000000000000";
  Mat output(1, 11, CV_32FC1); // �������0~9 X ʮһ�ֽ��
  Mat char_feature;
  Point maxLoc;
  double maxVal;

  // ǰ17������
  for (int i = 0; i < char_Mat.size() - 1; ++i) {
    if(char_Mat[i].data){
    calcGradientFeat(char_Mat[i], char_feature); // �����ַ�����
    ann->predict(char_feature, output);          // Ԥ��

    // Ѱ�������Сֵ����λ�ã������ֱ�ΪѰ�ҷ�Χ����Сֵ�����ֵ����Сֵλ�á����ֵλ��
    minMaxLoc(output, 0, &maxVal, 0, &maxLoc); // ��������е���ֵ������ȷ����ĸ��ʣ�Ѱ��������
    idResult[i] = (char)(maxLoc.x + '0');
    }
  }

  // ���һ������
  calcGradientFeat(char_Mat[17], char_feature); // �����ַ�����
  annX->predict(char_feature, output);          // Ԥ��

  // Ѱ�������Сֵ����λ�ã������ֱ�ΪѰ�ҷ�Χ����Сֵ�����ֵ����Сֵλ�á����ֵλ��
  minMaxLoc(output, 0, &maxVal, 0, &maxLoc); // ��������е���ֵ������ȷ����ĸ��ʣ�Ѱ��������
  if (maxLoc.x == 10)
	  idResult[17] = 'X';
  else idResult[17] = (char)(maxLoc.x + '0');

  return idResult;
}

// ͳ�Ʋ���ӡʶ����
void recognitionRate(const string &inputString, string &char_result, int numb) {
  int count = 0;
  string correct_result = inputString;

  // ͳ��
  for (int i = 0; i < 18; i++)
    if (correct_result[i] == char_result[i])
      count++;

  cout << "��ȷ�����";
  for (int i = 0; i < 18; i++)
    cout << correct_result[i] << " ";
  cout << endl;

  cout << "ʶ������";
  for (int i = 0; i < 18; i++) 
      cout << char_result[i] << " ";

  cout << endl;

  cout << endl;
  cout << "��ţ�" << numb << endl;
  cout << "���ַ���18" << endl;
  cout << "��ȷ�ַ���" << count << endl;
  cout << "ʶ���ʣ�" << count * 1.0 / 18 * 100 << "%" << endl;
}


// ���֤ʶ��
string identifyIdCard(const Mat &inputImage) {
	Mat ImageGray = getResizeGrayImage(inputImage); // ��ȡ�Ҷ�ͼ,����ͼ��С

	vector<RotatedRect> rects;                  // ƽ���ϵ���ת����
	posDetect(ImageGray, rects);                // ���֤���붨λ�����Ӻ��߿�
	Mat result;
	if (rects.size() != 0)
	  result = Cut_Area(ImageGray, rects[0]); // �ü����֤����

	vector<Mat> dst_mat;
	splitCharacter(result, dst_mat); // �и������ַ�

	// ���֤ʶ��
	Ptr<ANN_MLP> ann = ANN_MLP::load("ann/ann_param");    // ����������-����֪��
	Ptr<ANN_MLP> annX = ANN_MLP::load("ann/ann_param_X"); // ����������-����֪��

	string idResult = classifyCharacter(ann, annX, dst_mat); // ���������ַ�
	return idResult;
}
