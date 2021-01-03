#include "headers.h"

// ��ȡѵ������ͱ�ǩ���󣬲������xml�ļ�
void getAnnXML() {
  FileStorage fs("ann_xml.xml", FileStorage::WRITE);
  if (!fs.isOpened()) {
    cout << "Failed to open xml file." << endl;
  }

  Mat trainData;                             // ѵ������
  Mat classes = Mat::zeros(550, 1, CV_8UC1); // ��ǩ����,����0-9��X ÿ��50������
  Mat img_read;
  Mat dst_feature; // ���ѵ������������
  char path[90];   // ���ѵ���������ļ�·��

  // X
  for (int i = 0; i < 11; i++) {
    for (int j = 1; j <= 50; ++j) {
      sprintf_s(path, "Number_char/%d/%d (%d).png", i, i, j);
      img_read = imread(path, 1);
      cvtColor(img_read, img_read, COLOR_BGR2GRAY); // RGBת�Ҷ�
      calcGradientFeat(img_read, dst_feature);      // ��������������

      trainData.push_back(dst_feature);      // ������ȡ��ѵ������
      classes.at<uchar>(i * 50 + j - 1) = i; // ��ǩ��ȡ����ǩ����
    }
  }
  cout << "�������㼰��ȡ��ɣ�" << endl;

  // ѵ������ͱ�ǩ����д��xml�ļ�
  fs << "TrainingData" << trainData;
  fs << "classes" << classes;
  fs.release(); // ��������󣬲��ر��ļ�

  cout << "ѵ������ͱ�ǩ����д��xml�ļ���ɣ�" << endl;
}

// ��������������
void calcGradientFeat(const Mat &imgSrc, Mat &out) {
  vector<float> feat;

  // �ڶ�������-�Ҷ�ֵ��
  // 8 * 2������ֵ
  Mat image;
  resize(imgSrc, image, Size(8, 16)); // ͼƬͳһ��С��8��16��

  // �˲�ȥ�룬x�����y����
  float mask[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}}; // Boxģ��
  Mat y_mask = Mat(3, 3, CV_32F, mask) / 8;                // ��8��Ϊ�˹�һ��
  Mat x_mask = y_mask.t();                                 // ת��

  // ������㣬�����ֱ�Ϊ������ͼ�����ͼ��Ŀ��ͼ����ȡ������
  Mat sobelX, sobelY;
  filter2D(image, sobelX, CV_32F, x_mask);
  filter2D(image, sobelY, CV_32F, y_mask);

  sobelX = abs(sobelX);
  sobelY = abs(sobelY);

  // ��������ͼ��ĻҶ�ֵ֮��
  float totleValueX = sumMatValue(sobelX);
  float totleValueY = sumMatValue(sobelY);

  // ͼ�񻮷�Ϊ2*4��8�����ӣ�ÿ������4*4�����ص㣬����ÿ�����ӻҶ�ֵ����ͼ��Ҷ�ֵ֮��
  Mat subImageX;
  Mat subImageY;
  for (int i = 0; i < image.rows; i = i + 4) {
    for (int j = 0; j < image.cols; j = j + 4) {
      subImageX = sobelX(Rect(j, i, 4, 4));
      feat.push_back(sumMatValue(subImageX) / totleValueX);
      subImageY = sobelY(Rect(j, i, 4, 4));
      feat.push_back(sumMatValue(subImageY) / totleValueY);
    }
  }

  // ��һ������-�Ҷ�ֵ��
  // 32������ֵ
  Mat imageGray;
  cv::resize(imgSrc, imageGray, Size(4, 8)); // ͼƬͳһ��С��4��8��
  Mat p = imageGray.reshape(1, 1);           // reshape(int cn, int rows=0) cn��ʾͨ��������rows��ʾ��������
  p.convertTo(p, CV_32FC1);                  // ����ת��

  for (int i = 0; i < p.cols; i++) {
    feat.push_back(p.at<float>(i));
  }

  // ����������-ˮƽ����ֱֱ��ͼ
  // 8+16������
  Mat vhist = projectHistogram(imgSrc, 1); // ˮƽֱ��ͼ
  Mat hhist = projectHistogram(imgSrc, 0); // ��ֱֱ��ͼ
  for (int i = 0; i < vhist.cols; i++)
    feat.push_back(vhist.at<float>(i));
  for (int i = 0; i < hhist.cols; i++)
    feat.push_back(hhist.at<float>(i));

  // д��
  out = Mat::zeros(1, (int)feat.size(), CV_32F);
  for (int i = 0; i < feat.size(); i++)
    out.at<float>(i) = feat[i];
}

// ����Ҷ�ֵ��
float sumMatValue(const Mat &image) {
  float sumValue = 0;
  int r = image.rows;
  int c = image.cols;
  if (image.isContinuous()) // ���ڴ�����������Ϊһ�ж���
  {
    c = r * c;
    r = 1;
  }

  for (int i = 0; i < r; i++) {
    const uchar *linePtr = image.ptr<uchar>(i);
    for (int j = 0; j < c; j++) {
      sumValue += linePtr[j];
    }
  }

  return sumValue;
}

// ����ֱ��ͼ
Mat projectHistogram(const Mat &img, int t) {
  Mat lowData;
  cv::resize(img, lowData, Size(8, 16));
  int sz = (t) ? lowData.rows : lowData.cols; // t=1����ˮƽ��t=0���㴹ֱ
  Mat mhist = Mat::zeros(1, sz, CV_32F);

  for (int j = 0; j < sz; j++) {
    Mat data = (t) ? lowData.row(j) : lowData.col(j);
    mhist.at<int>(j) = countNonZero(data); // ���ػҶ�ֵ��Ϊ0�����������ڵװ��֣����������ֵ����ص�����
  }

  double min, max;
  minMaxLoc(mhist, &min, &max); // Ѱ�������Сֵ
  // ��������ת���������ֱ�Ϊ��Ŀ�����Ŀ������Դ������������һ�£��߶ȱ任���ӣ���һ�������߶ȱ任���ƫ����
  if (max > 1)
    mhist.convertTo(mhist, -1, 1.0f / max, 0);

  return mhist;
}

// ѵ��������
void trainAnn(int nlayers, int numCharacters) {
  Ptr<ANN_MLP> ann = ANN_MLP::create(); // ����������-����֪��

  // ���ļ���ȡѵ�����󣬱�ǩ����
  Mat trainData, classes;
  FileStorage fs;
  fs.open("ann_xml.xml", FileStorage::READ);
  fs["TrainingData"] >> trainData;
  fs["classes"] >> classes;

  Mat layerSizes(1, 3, CV_32SC1);         // 3��������
  layerSizes.at<int>(0) = trainData.cols; // ��������Ԫ�����-ÿ����������������
  layerSizes.at<int>(1) = nlayers;        // ���ز����Ԫ�����-
  layerSizes.at<int>(2) = numCharacters;  // ��������Ԫ�����-0~9,X

  ann->setLayerSizes(layerSizes);
  ann->setTrainMethod(ANN_MLP::BACKPROP, 0.1, 0.1);                       // ���򴫲��㷨��Ȩ�ݶ���ǿ�ȣ�һ��Ϊ0.1����������ǿ�ȣ�һ��Ϊ0.1��
  ann->setActivationFunction(ANN_MLP::SIGMOID_SYM);                       // �����
  ann->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 5000, 0.01)); // �����㷨����ֹ׼�򣬺��������Ϊ�����������ͽ����ȷ��

  // Ϊ���������ϱ�ǩ
  Mat trainClasses;
  trainClasses.create(trainData.rows, numCharacters, CV_32FC1);
  for (int i = 0; i < trainData.rows; i++) // ÿһ��ѵ������
  {
    for (int k = 0; k < trainClasses.cols; k++) // ÿһ�ֿ��ܵĽ��
    {
      if (k == (int)classes.at<uchar>(i)) // ѵ�������ı�ǩ������Ӧ
      {
        trainClasses.at<float>(i, k) = 1;
      }

      else
        trainClasses.at<float>(i, k) = 0;
    }
  }

  cout << "����ѵ��������" << endl;
  ann->train(trainData, ml::ROW_SAMPLE, trainClasses);
  cout << "ѵ����ɣ�����" << endl;
  ann->save("/ann/ann_param_X"); // ����MLPΪ��ִ���ļ�
}
