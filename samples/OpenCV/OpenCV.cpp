// OpenCV.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "ovrvision_pro.h"

using namespace cv;
using namespace OVR;

static 	OvrvisionPro ovrvision;

int main(int argc, char* argv[])
{
	if (ovrvision.Open(0, Camprop::OV_CAMHD_FULL))
	{
		int width = ovrvision.GetCamWidth();
		int height = ovrvision.GetCamHeight();
		cv::Mat left(height, width, CV_8UC4);
		cv::Mat right(height, width, CV_8UC4);

		//Sync
		ovrvision.SetCameraSyncMode(true);

		Camqt mode = Camqt::OV_CAMQT_DMSRMP;
		bool show = true;

		for (bool loop = true; loop;)
		{
			//DWORD begin = GetTickCount();
			if (show)
			{
				// Capture frame
				ovrvision.PreStoreCamData(mode);

				// Retrieve frame data
				ovrvision.GetCamImageBGRA(left.data, Cameye::OV_CAMEYE_LEFT);
				ovrvision.GetCamImageBGRA(right.data, Cameye::OV_CAMEYE_RIGHT);

				// ここでOpenCVでの加工など

				// Show frame data
				imshow("Left", left);
				imshow("Right", right);
			}
			else
			{
				ovrvision.Capture(mode);
			}

			//DWORD end = GetTickCount();
			//printf("%f fps %d ms/frame\n", 1000.0f / (end - begin), (end - begin));

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

			case 's':
				show = true;
				break;

			case 'n':
				show = false;
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


