#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <libfreenect2_frame_grabber.hpp>

#define DEPTHWIDTH  (512)
#define DEPTHHEIGHT (424)
//#define COLORWIDTH    (1920)
//#define COLORHEIGHT   (1080)
#define COLORWIDTH    (DEPTHWIDTH)
#define COLORHEIGHT   (DEPTHHEIGHT)
#define NEARMODE    (true)
//#define REGISTERED  (false)
#define REGISTERED  (true)
#define DEPTHFPS    (30)
#define COLORFPS    (30)


int main(int argc, char *argv[])
{
  FrameGrabberParams params = {1, DEPTHWIDTH, DEPTHHEIGHT, DEPTHFPS,
			       COLORWIDTH, COLORHEIGHT, COLORFPS,
			       NEARMODE, false, REGISTERED, false};
  Libfreenect2FrameGrabber grabber(params);
  
  cv::namedWindow("test_depth");
  cv::namedWindow("test_color");

  sleep(2);

  while (true)
  {
    //cv::Mat depth(DEPTHHEIGHT, DEPTHWIDTH, CV_16U, (void*)grabber.getDepth());
    //cv::Mat color(COLORHEIGHT, COLORWIDTH, CV_8UC3, (void*)grabber.getColor());
    cv::Mat depth(DEPTHHEIGHT, DEPTHWIDTH, CV_16U);
    cv::Mat color(COLORHEIGHT, COLORWIDTH, CV_8UC4);
    
    grabber.copyDepth(reinterpret_cast<unsigned short*>(depth.data));
    grabber.copyColor(reinterpret_cast<unsigned char*>(color.data));

    cv::imshow("test_depth", depth);
    cv::imshow("test_color", color);

    char key = cv::waitKey(1);
    if (key=='q')
    {
      break;
    }
  }

  cv::destroyAllWindows();

  return 0;
}
