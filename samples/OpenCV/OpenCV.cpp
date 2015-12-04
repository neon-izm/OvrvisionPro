// OpenCV.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ovrvision_pro.h"

#define CROP_W 1024
#define CROP_H 768

//using namespace std;
using namespace cv;
using namespace OVR;

static 	OvrvisionPro ovrvision;

int callback(void *pItem, const char *extensions)
{
	if (extensions != NULL)
	{
		puts(extensions);
	}
	return 0;
}

// Estimate skin color 
void estimateSkincolor(Mat &histgram, OvrvisionPro &camera, ROI roi)
{
	Mat left(roi.height, roi.width, CV_8UC4);
	Mat right(roi.height, roi.width, CV_8UC4);
	Mat images[2];
	images[0].create(roi.height, roi.width, CV_8UC4);
	images[1].create(roi.height, roi.width, CV_8UC4);

	int channels[] = { 0, 1 }; // H, S
	int histSize[] = { 256, 256 };
	float hranges[] = { 0, 256 };
	float sranges[] = { 0, 256 };
	const float* ranges[] = { hranges, sranges };

	while (waitKey(10) != 'a')
	{
		// Capture frame
		ovrvision.Capture(Camqt::OV_CAMQT_DMSRMP);

		// Retrieve frame data
		ovrvision.GetStereoImageBGRA(left.data, right.data, roi);
		imshow("Left", left);
		imshow("Right", right);
	}

	// back ground
	Mat background(256, 256, CV_32FC1);
	background.setTo(Scalar::all(0));
	for (int i = 0; i < 50; i++)
	{
		// Capture frame
		ovrvision.Capture(Camqt::OV_CAMQT_DMSRMP);

		// Retrieve frame data
		ovrvision.GetStereoImageBGRA(left.data, right.data, roi);
		cvtColor(left, images[0], CV_BGR2HSV_FULL);
		cvtColor(right, images[1], CV_BGR2HSV_FULL);
		calcHist(images, 2, channels, Mat(), background, 2, histSize, ranges, true, true);

		imshow("Left", left);
		imshow("Right", right);
	}
	while (waitKey(10) != 'a')
	{
		// Capture frame
		ovrvision.Capture(Camqt::OV_CAMQT_DMSRMP);

		// Retrieve frame data
		ovrvision.GetStereoImageBGRA(left.data, right.data, roi);

		putText(left, "Waiting...", Point(0, 50), cv::FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 255), 2);
		putText(right, "Waiting...", Point(0, 50), cv::FONT_HERSHEY_PLAIN, 3.0, Scalar(0, 255, 255), 2);
		imshow("Left", left);
		imshow("Right", right);
	}

	// weave hands
	Mat foreground(256, 256, CV_32FC1);
	foreground.setTo(Scalar::all(0));
	for (int i = 0; i < 50; i++)
	{
		// Capture frame
		ovrvision.Capture(Camqt::OV_CAMQT_DMSRMP);

		// Retrieve frame data
		ovrvision.GetStereoImageBGRA(left.data, right.data, roi);
		cvtColor(left, images[0], CV_BGR2HSV_FULL);
		cvtColor(right, images[1], CV_BGR2HSV_FULL);
		calcHist(images, 2, channels, Mat(), foreground, 2, histSize, ranges, true, true);

		putText(left, "Initiating...", Point(0, 50), cv::FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 255), 2);
		putText(right, "Initiating...", Point(0, 50), cv::FONT_HERSHEY_PLAIN, 3.0, Scalar(0, 255, 255), 2);
		imshow("Left", left);
		imshow("Right", right);
	}
	
	// Compare histgram
	Mat difference(256, 256, CV_32FC1);
	difference.setTo(Scalar::all(0));
	float maxVal = 0;
	for (int h = 3; h < 256; h++)
	{
		float *b = background.ptr<float>(h);
		float *f = foreground.ptr<float>(h);
		float *d = difference.ptr<float>(h);
		for (int s = 0; s < 256; s++)
		{
			float diff = f[s] - b[s];
			if (0 < diff)
			{
				d[s] = diff;
				if (maxVal < diff)
					maxVal = diff;
			}
		}
	}
	Mat diff(256, 256, CV_32FC1);
	GaussianBlur(difference, diff, Size(3, 3), 8, 6);
	//normalize(background, histgram, 0, 255, NORM_MINMAX, histgram.type());
	//imshow("background", histgram);
	//normalize(foreground, histgram, 0, 255, NORM_MINMAX, histgram.type());
	//imshow("foreground", histgram);
	normalize(diff, histgram, 0, 255, NORM_MINMAX, histgram.type());
	imshow("histgram", histgram);
	imwrite("histgram.png", histgram);
}

enum FILTER {
	GAUSSIAN,
	MEDIAN,
	NONE,
};

int main(int argc, char* argv[])
{
	if (ovrvision.Open(0, Camprop::OV_CAMHD_FULL))
	{
		int ksize = 5;
		enum FILTER filter = GAUSSIAN;
		int width = ovrvision.GetCamWidth();
		int height = ovrvision.GetCamHeight();

		Mat l(height / 2, width / 2, CV_8UC4);
		Mat r(height / 2, width / 2, CV_8UC4);
		ROI roi = {(width - CROP_W) / 2, (height - CROP_H) / 2, CROP_W, CROP_H};
		Mat left(roi.height, roi.width, CV_8UC4);
		Mat right(roi.height, roi.width, CV_8UC4);
		Mat histgram(256, 256, CV_8UC1);

		Mat LEFT(roi.height / 2, roi.width / 2, CV_8UC4);
		Mat RIGHT(roi.height / 2, roi.width / 2, CV_8UC4);
		Mat Lhsv(roi.height / 2, roi.width / 2, CV_8UC3);
		Mat Rhsv(roi.height / 2, roi.width / 2, CV_8UC3);
		Mat lHSV(roi.height / 2, roi.width / 2, CV_8UC3);
		Mat rHSV(roi.height / 2, roi.width / 2, CV_8UC3);
		Mat Lresult(roi.height / 2, roi.width / 2, CV_8UC4);
		Mat Rresult(roi.height / 2, roi.width / 2, CV_8UC4);
		Mat bilevel_l(roi.height / 2, roi.width / 2, CV_8UC1);
		Mat bilevel_r(roi.height / 2, roi.width / 2, CV_8UC1);
		//Mat blur(roi.height / 2, roi.width / 2, CV_8UC4);
		//Mat blur2(roi.height / 2, roi.width / 2, CV_8UC4);

		std::vector<std::vector<Point>> contours;
		std::vector<Vec4i> hierarchy;

		//Sync
		ovrvision.SetCameraSyncMode(true);

		/*
		Mat P1, P2, T, Q;
		FileStorage cvfs;
		if (cvfs.open("epipolar.xml", CV_STORAGE_READ | CV_STORAGE_FORMAT_XML))
		{
			//get data node
			FileNode data(cvfs.fs, NULL);
			data["P1"] >> P1;
			data["P2"] >> P2;
			data["T"] >> T;
			data["Q"] >> Q;
			cvfs.release();
			printf("T =[ %f %f %f ]\n", T.at<double>(0, 0), T.at<double>(0, 1), T.at<double>(0, 2));
			printf("Q[3] = [ %f %f %f %f ]\n", Q.at<double>(0, 3), Q.at<double>(1, 3), Q.at<double>(2, 3), Q.at<double>(3, 3));
		}
		*/

		Camqt mode = Camqt::OV_CAMQT_DMSRMP;
		bool show = true;
		bool useHistgram = false;

		// Read histgram
		histgram = imread("histgram.bmp");
		if (histgram.empty())
		{
			//estimateSkincolor(histgram, ovrvision, roi);
		}
		else
		{
			//useHistgram = true;
			imshow("histgram", histgram);
			/*
			for (int y = 0; y < 256; y += 4)
			{
				printf("H:%03d, ", y);
				uchar *row = histgram.ptr<uchar>(y);
				for (int x = 0; x < 256; x += 4)
				{
					printf("%d, ", row[x]);
				}
				puts("");
			}
			*/
		}

		for (bool loop = true; loop;)
		{
			if (show)
			{
				// Capture frame
				ovrvision.Capture(mode);

				// Retrieve frame data
				ovrvision.GetStereoImageBGRA(left.data, right.data, roi);

				// ここでOpenCVでの加工など
				if (0 < ksize)
				{
					// ここからGPU（OpenCL化）
					resize(left, LEFT, LEFT.size());
					resize(right, RIGHT, RIGHT.size());

					//
					switch (filter)
					{
					case GAUSSIAN:
						cvtColor(LEFT, Lhsv, CV_BGR2HSV_FULL);
						cvtColor(RIGHT, Rhsv, CV_BGR2HSV_FULL);
						GaussianBlur(Lhsv, lHSV, Size(ksize, ksize), 0);
						GaussianBlur(Rhsv, rHSV, Size(ksize, ksize), 0);
						break;

					case MEDIAN:
						cvtColor(LEFT, Lhsv, CV_BGR2HSV_FULL);
						cvtColor(RIGHT, Rhsv, CV_BGR2HSV_FULL);
						medianBlur(Lhsv, lHSV, ksize);
						medianBlur(Rhsv, rHSV, ksize);
						break;

					default:
						cvtColor(LEFT, lHSV, CV_BGR2HSV_FULL);
						cvtColor(RIGHT, rHSV, CV_BGR2HSV_FULL);
						break;
					}
					
					Lresult.setTo(Scalar(0, 0, 0, 255));
					Rresult.setTo(Scalar(0, 0, 0, 255));
					bilevel_l.setTo(Scalar::all(0));
					bilevel_r.setTo(Scalar::all(0));
					for (uint y = 0; y < roi.height / 2; y++)
					{
						Vec3b *l = lHSV.ptr<Vec3b>(y);
						Vec3b *r = rHSV.ptr<Vec3b>(y);
						Vec4b *Lpixel = Lresult.ptr<Vec4b>(y);
						Vec4b *Rpixel = Rresult.ptr<Vec4b>(y);
						uchar *b_l = bilevel_l.ptr<uchar>(y);
						uchar *b_r = bilevel_r.ptr<uchar>(y);
						for (uint x = 0; x < roi.width / 2; x++)
						{
							uchar h = l[x][0];
							uchar s = l[x][1];
							if (useHistgram)
							{
								if (1 < histgram.at<uchar>(h, s))
								{
									Lpixel[x] = LEFT.at<Vec4b>(y, x);
									b_l[x] = 255;
								}
								h = r[x][0];
								s = r[x][1];
								if (3 < histgram.at<uchar>(h, s))
								{
									Rpixel[x] = RIGHT.at<Vec4b>(y, x);
									b_r[x] = 255;
								}
							}
							else
							{
								if (15 <= h && h <= 30 && 55 < s && s < 150)
								{
									Lpixel[x] = LEFT.at<Vec4b>(y, x);
									b_l[x] = 255;
								}
								h = r[x][0];
								s = r[x][1];
								if (15 <= h && h <= 30 && 55 < s && s < 150)
								{
									Rpixel[x] = RIGHT.at<Vec4b>(y, x);
									b_r[x] = 255;
								}
							}
						}
					}
					// ここまでGPU（OpenCL）で

					// CPU側（OpenCV）
					findContours(bilevel_r, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
					for (uint i = 0; i < contours.size(); i++)
					{
						if (200 < contours[i].size())
							drawContours(Rresult, contours, i, Scalar(255, 255, 255), 1, 8);
					}
					findContours(bilevel_l, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
					for (uint i = 0; i < contours.size(); i++)
					{
						if (200 < contours[i].size())
							drawContours(Lresult, contours, i, Scalar(255, 255, 255), 1, 8);
					}
					// ここまでOpenCVで処理してGPUに戻す

					// Show frame data
					imshow("bilevel(L)", bilevel_l);
					imshow("bilevel(R)", bilevel_r);
					imshow("Left", LEFT);
					imshow("Right", RIGHT);
					imshow("L", Lresult);
					imshow("R", Rresult);
					//imshow("Blur(L)", blur);
					//imshow("Blur(R)", blur2);
					//imshow("Lhsv", Lhsv);
				}
				else
				{
					// Show frame data
					imshow("Left", left);
					imshow("Right", right);
				}
			}
			else
			{
				ovrvision.Capture(mode);
				ROI roi = { 50, 0, CROP_W, CROP_H };
				//ovrvision.GetStereoImageBGRA(left.data, right.data, roi);
				ovrvision.Read(l.data, r.data);
				
				imshow("Left", l);
				imshow("Right", r);
			}

			switch (waitKey(1))
			{
			case 'q':
				loop = false;
				break;

			case 'r':
				mode = Camqt::OV_CAMQT_DMSRMP;
				break;

			case 'd':
				mode = Camqt::OV_CAMQT_DMS;
				break;

			case 'h':
				useHistgram = !useHistgram;
				break;

			case 's':
				show = !show;
				break;

			case 'g':
				filter = GAUSSIAN;
				break;

			case 'm':
				filter = MEDIAN;
				break;

			case 'n':
				filter = NONE;
				break;

			case '0':
			case '1':
				ksize = 0;
				break;

			case '3':
				ksize = 3;
				break;

			case '5':
				ksize = 5;
				break;

			case '7':
				ksize = 7;
				break;

			case '9':
				ksize = 9;
				break;

			case ' ':
				if (show)
				{
					imwrite("left.png", left);
					imwrite("right.png", right);
					imwrite("hsv_L.bmp", lHSV);
					imwrite("hsv_R.bmp", rHSV);
					imwrite("hsv_l.png", Lhsv);
					imwrite("hsv_r.png", Rhsv);
					imwrite("result_l.png", Lresult);
					imwrite("result_r.png", Rresult);
					//imwrite("blur_l.png", blur);
					//imwrite("blur_r.png", blur2);
					//imwrite("histgram.png", histgram);
				}
				else
				{
					imwrite("LEFT.tiff", left);
					imwrite("RIGHT.tiff", right);
				}
				break;

			case 'e':
				if (callback != NULL)
				{
					callback(NULL, "Device Extensions");
				}
				ovrvision.OpenCLExtensions(callback, NULL);
				break;
			}
		}
	}
	else
	{
		puts("FAILED TO OPEN CAMERA");
	}
	return 0;
}


