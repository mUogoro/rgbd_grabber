//***************************************************************************/
// test_openni2.cpp
//
// Test the OpenNI2 interface
//
//***************************************************************************/

// Include OpenCV headers for stream processing
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Include the OpenNI2 interface header
#include <openni2_grabber.hpp>

using namespace cv;

// Some macros for preferred camera settings
#define DEPTHWIDTH  (640)
#define DEPTHHEIGHT (480)
#define COLORWIDTH  (640)
#define COLORHEIGHT (480)
#define NEARMODE    (true)
#define DEPTHFPS    (30)
#define COLORFPS    (30)

int main(int argc, char *argv[])
{
  // RGBD camera parameters structure. It will be passed to the camera-specific
  // interface constructor
  RGBDGrabberParams params = {
    0,            // Device ID (0 first device, 1 second device etc.)
    DEPTHWIDTH,   // Depth stream width
    DEPTHHEIGHT,  // Depth stream height
    DEPTHFPS,     // Depth stream frames-per-second
    COLORWIDTH,   // Color stream width
    COLORHEIGHT,  // Color stream height
    COLORFPS,     // Color stream frames-per-second
    NEARMODE,     // Near mode (i.e. close interaction working mode)
    false,        // Register (i.e. align) depth stream over color stream 
    false,        // Register (i.e. align) color stream over depth stream
    false         // Enable streams mirroring
  };

  // Init the camera interface instance
  OpenNI2Grabber grabber(params);

  namedWindow("test_depth");
  namedWindow("test_color");

  // Acquisition loop
  while (true)
  {
    // Internal stream retrieval mode:
    // retrive directly the pointer to the internal depth and color stream
    // Note: the pointer is not guaranted to be valid after the call. For this
    // reason this mode is HIGHLY discouraged
    /*
    unsigned short *depthPtr = grabber.getDepth();
    unsigned char *colorPtr = grabber.getColor();
    */

    // Stream copy mode:
    // copy the currently retrieved frame. This mode guarantes that the returned
    // frame will be valid after the call
    // Note: in copy mode the timestamp can be retrieved as well
    unsigned short *depthPtr = new unsigned short[DEPTHWIDTH*DEPTHHEIGHT];
    unsigned char *colorPtr = new unsigned char[COLORWIDTH*COLORHEIGHT*3];
    unsigned long int depthTimestamp, colorTimestamp;
    grabber.copyDepth(depthPtr, &depthTimestamp);
    grabber.copyColor(colorPtr, &colorTimestamp);

    // Wrap the retrieved frames into OpenCV Mats and show them
    Mat depth(DEPTHHEIGHT, DEPTHWIDTH, CV_16U, static_cast<void*>(depthPtr));
    Mat rgb(COLORHEIGHT, COLORWIDTH, CV_8UC3, static_cast<void*>(colorPtr));
    Mat bgr(COLORHEIGHT, COLORWIDTH, CV_8UC3);
    cv::cvtColor(rgb, bgr, CV_RGB2BGR);
    imshow("test_depth", depth);
    imshow("test_color", bgr);

    // Done, cleanup and wait for 'q' key pressed
    delete []depthPtr;
    delete []colorPtr;

    char key = waitKey(1);
    if (key=='q')
    {
      break;
    }
  }

  destroyAllWindows();

  return 0;
}
