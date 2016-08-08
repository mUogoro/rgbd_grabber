//***************************************************************************/
// test_softkinetic.cpp
//
// Test the SoftKinetic interface
// 
// Note: for more details about the RGBGrabber library interface please
// look at the test_openi2.cpp example
//
//***************************************************************************/
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <softkinetic_grabber.hpp>

using namespace cv;

#define DEPTHWIDTH  (320)
#define DEPTHHEIGHT (240)
#define COLORWIDTH    (640)
#define COLORHEIGHT   (480)
#define NEARMODE    (true)
#define DEPTHFPS    (25)
#define COLORFPS    (25)

int main(int argc, char *argv[])
{
  RGBDGrabberParams params = {0, DEPTHWIDTH, DEPTHHEIGHT, DEPTHFPS,
			       COLORWIDTH, COLORHEIGHT, COLORFPS,
			       NEARMODE, false, false, false};
  SoftkineticGrabber grabber(params);

  namedWindow("test_depth");
  namedWindow("test_color");

  unsigned short *depthPtr = new unsigned short[DEPTHWIDTH*DEPTHHEIGHT];
  unsigned char *colorPtr = new unsigned char[COLORWIDTH*COLORHEIGHT*3];
  unsigned long int depthTimestamp, colorTimestamp;
  while (true)
  {
    grabber.copyDepth(depthPtr, &depthTimestamp);
    grabber.copyColor(colorPtr, &colorTimestamp);

    Mat depth(DEPTHHEIGHT, DEPTHWIDTH, CV_16U, static_cast<void*>(depthPtr));
    Mat color(COLORHEIGHT, COLORWIDTH, CV_8UC3, static_cast<void*>(colorPtr));
    imshow("test_depth", depth);
    imshow("test_color", color);

    waitKey(1);
  }

  delete []colorPtr;
  delete []depthPtr;

  destroyAllWindows();

  return 0;
}
