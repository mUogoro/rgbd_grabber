#include <vector>
#include <cmath>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <softkinetic_grabber.hpp>

#ifdef WIN32
#include <Windows.h>
#define sleep(__ms) Sleep((__ms)*1.e3)
#else
#include <unistd.h>
#endif // WIN32

using namespace std;


#define CHECK_SOFTKINETIC_CALL(__CONDITION, __ERROR_STRING)\
{\
  if (!(__CONDITION))\
  {\
    std::stringstream errss;\
    errss << __ERROR_STRING << std::endl;\
    throw std::runtime_error(errss.str());\
  }\
}


static void _onNewDepthSample(DepthNode node, DepthNode::NewSampleReceivedData data, SoftkineticGrabber *self);
static void _onNewColorSample(ColorNode node, ColorNode::NewSampleReceivedData data, SoftkineticGrabber *self);
static void *_acquisitionLoop(void *self);

SoftkineticGrabber::SoftkineticGrabber(RGBDGrabberParams &params):
  depthWidth(params.depthWidth), depthHeight(params.depthHeight),
  colorWidth(params.colorWidth), colorHeight(params.colorHeight),
  depthFPS(params.depthFPS), colorFPS(params.colorFPS),
  nearMode(params.nearMode)
{
  if (params.regDepthOverColor || params.regColorOverDepth)
  {
    CHECK_SOFTKINETIC_CALL(false, "Registration not supported");
  }

  vector<Device> devs;
  vector<Node> nodes;
  DepthNode::Configuration depthConfig;
  ColorNode::Configuration colorConfig;
  StereoCameraParameters cameraParams;
  bool depthFound=false, colorFound=true;

  context = Context::create("localhost");
  
  devs = context.getDevices();
  CHECK_SOFTKINETIC_CALL(devs.size()>=1, "No device found");
  
  nodes = devs[0].getNodes();
  for (int i=0; i<nodes.size(); i++)
  {
    if (nodes[i].is<DepthNode>())
    {
      depthNode = nodes[i].as<DepthNode>();
      depthFound = true;
    }
    else if (nodes[i].is<ColorNode>())
    {
      colorNode = nodes[i].as<ColorNode>();
      colorFound = true;
    }
    if (depthFound && colorFound) break;
  }

  CHECK_SOFTKINETIC_CALL(depthFound && colorFound,
			 "Unable to find both depth and color streams");

  // Read camera focal lengths and compute corresponding FOVs
  cameraParams = devs[0].getStereoCameraParameters();

  depthHFOV = 2.0f*atan(cameraParams.depthIntrinsics.width/
			(2.0f*cameraParams.depthIntrinsics.fx));
  depthHFOV *= 180.0f/M_PI;
  depthVFOV = 2.0f*atan(cameraParams.depthIntrinsics.height/
			(2.0f*cameraParams.depthIntrinsics.fy));
  depthVFOV *= 180.0f/M_PI;

  colorHFOV = 2.0f*atan(cameraParams.colorIntrinsics.width/
			(2.0f*cameraParams.colorIntrinsics.fx));
  colorHFOV *= 180.0f/M_PI;
  colorVFOV = 2.0f*atan(cameraParams.colorIntrinsics.height/
			(2.0f*cameraParams.colorIntrinsics.fy));
  colorVFOV *= 180.0f/M_PI;


  depthNode.newSampleReceivedEvent().connect<SoftkineticGrabber*>(&_onNewDepthSample, this);
  colorNode.newSampleReceivedEvent().connect<SoftkineticGrabber*>(&_onNewColorSample, this);

  // So far only 320x240 resolution are supported
  // TODO: support more resolutions
  depthConfig = depthNode.getConfiguration();
  if (depthWidth==320 && depthHeight==240)
  {
    depthConfig.frameFormat = FRAME_FORMAT_QVGA;
  }
  else
  {
    CHECK_SOFTKINETIC_CALL(false, "Unsupported depth resolution specified. Only 320x240 supported.");
  }
  // TODO:
  // - configure framerate?
  // - test without saturation
  depthConfig.framerate=depthFPS;
  depthConfig.mode = (nearMode) ? DepthNode::CAMERA_MODE_CLOSE_MODE : DepthNode::CAMERA_MODE_LONG_RANGE;
  depthConfig.saturation = true;
  
  depthNode.setEnableDepthMap(true);

  context.requestControl(depthNode, 0);
  depthNode.setConfiguration(depthConfig);
  // TODO: handle exception here

  // TODO: same stuff as for depth node
  colorConfig = colorNode.getConfiguration();
  if (colorWidth==640 && colorHeight==480)
  {
    colorConfig.frameFormat = FRAME_FORMAT_VGA;
    colorConfig.compression = COMPRESSION_TYPE_MJPEG;
  }
  else if (colorWidth==320 && colorHeight==240)
  {
    colorConfig.compression = COMPRESSION_TYPE_YUY2;
    colorConfig.frameFormat = FRAME_FORMAT_QVGA;
  }
  else
  {
    CHECK_SOFTKINETIC_CALL(false, "Unsupported color resolution specified. Only 320x240 and 640x480 supported.");
  }

  colorConfig.powerLineFrequency = POWER_LINE_FREQUENCY_50HZ;
  colorConfig.framerate = colorFPS;

  colorNode.setEnableColorMap(true);

  context.requestControl(colorNode, 0);
  colorNode.setConfiguration(colorConfig);
  
  pthread_mutex_init(&depthMtx, NULL);
  pthread_mutex_init(&colorMtx, NULL);
  
  context.registerNode(depthNode);
  context.registerNode(colorNode);
  context.startNodes();

  depthPtr = NULL;
  colorPtr = NULL;
  depthTimestamp = 0;
  colorTimestamp = 0;

  pthread_create(&loopThread, NULL, _acquisitionLoop, (void*)this);

  // TODO: better way to wait for data
  // Spin lock on depth and color pointers
  //while (!depthPtr || !colorPtr);
  sleep(1);
}


SoftkineticGrabber::~SoftkineticGrabber()
{
  context.stopNodes();
  pthread_join(loopThread, NULL);

  context.unregisterNode(depthNode);
  context.unregisterNode(colorNode);

  context.quit();

  pthread_mutex_destroy(&depthMtx);
  pthread_mutex_destroy(&colorMtx);
}


unsigned int SoftkineticGrabber::getDepthWidth()
{
  return depthWidth;
}

unsigned int SoftkineticGrabber::getDepthHeight()
{
  return depthHeight;
}

unsigned int SoftkineticGrabber::getColorWidth()
{
  return colorWidth;
}

unsigned int SoftkineticGrabber::getColorHeight()
{
  return colorHeight;
}

float SoftkineticGrabber::getDepthHFOV()
{
  return depthHFOV;
}

float SoftkineticGrabber::getDepthVFOV()
{
  return depthVFOV;
}

float SoftkineticGrabber::getColorHFOV()
{
  return colorHFOV;
}

float SoftkineticGrabber::getColorVFOV()
{
  return colorVFOV;
}


const unsigned short *SoftkineticGrabber::getDepth(unsigned long *timestamp)
{
  if (timestamp) *timestamp = depthTimestamp;  
  return depthPtr;
}

const unsigned char *SoftkineticGrabber::getColor(unsigned long *timestamp)
{
  if (timestamp) *timestamp = colorTimestamp;
  return colorPtr;
}


void SoftkineticGrabber::copyDepth(unsigned short *depth, unsigned long *timestamp)
{
  pthread_mutex_lock(&depthMtx);
  memcpy(depth, depthPtr,
  	 sizeof(unsigned short)*depthWidth*depthHeight);
  if (timestamp) *timestamp = depthTimestamp;
  pthread_mutex_unlock(&depthMtx);
}

void SoftkineticGrabber::copyColor(unsigned char *color, unsigned long *timestamp)
{
  pthread_mutex_lock(&colorMtx);
  memcpy(color, colorPtr,
  	 sizeof(unsigned char)*colorWidth*colorHeight*3);
  if (timestamp) *timestamp = colorTimestamp;

  if (colorWidth==320 && colorHeight==240)
  {
    cv::Mat inColor(240, 320, CV_8UC2);
    memcpy(inColor.data, colorPtr, sizeof(unsigned char)*320*240*2);
    cv::Mat cvtColor(240, 320, CV_8UC3, color);
    cv::cvtColor(inColor, cvtColor, CV_YUV2RGB_YUYV);
  }

  pthread_mutex_unlock(&colorMtx);
}


void SoftkineticGrabber::_copyInternalDepthBuffer(const unsigned short *buffer, unsigned long timestamp)
{
  pthread_mutex_lock(&depthMtx);
  depthPtr = buffer;
  depthTimestamp = timestamp;
  pthread_mutex_unlock(&depthMtx);
}

void SoftkineticGrabber::_copyInternalColorBuffer(const unsigned char *buffer, unsigned long timestamp)
{
  pthread_mutex_lock(&colorMtx);
  colorPtr = buffer;
  colorTimestamp = timestamp;
  pthread_mutex_unlock(&colorMtx);
}


void _onNewDepthSample(DepthNode node, DepthNode::NewSampleReceivedData data, SoftkineticGrabber *self)
{
  self->_copyInternalDepthBuffer((const unsigned short*)&(*data.depthMap), data.timeOfArrival);
}

void _onNewColorSample(ColorNode node, ColorNode::NewSampleReceivedData data, SoftkineticGrabber *self)
{
  self->_copyInternalColorBuffer((const unsigned char*)&(*data.colorMap), data.timeOfArrival);
}


void SoftkineticGrabber::_runContext()
{
  context.run();
}

void *_acquisitionLoop(void *_self)
{
  SoftkineticGrabber *self = (SoftkineticGrabber*)_self;
  self->_runContext();
  return NULL;
}
