#include <cmath>
#include <iostream>
#include <algorithm>
#include <openni2_grabber.hpp>
#include <sstream>
#include <stdexcept>

#ifdef WIN32
#include <Windows.h>
#define sleep(__ms) Sleep((__ms)*1.e3)
#else
#include <unistd.h>
#endif // WIN32

using namespace openni;


#define CHECK_OPENNI_CALL(__STATUS, __ERROR_STRING) \
{\
  if ((__STATUS)!=STATUS_OK)\
  {\
    std::stringstream errss;\
    errss << (__ERROR_STRING) << ": " << OpenNI::getExtendedError() << std::endl;\
    throw std::runtime_error(errss.str());\
  }\
}


static void* _acquisitionLoop(void *self);


OpenNI2Grabber::OpenNI2Grabber(RGBDGrabberParams &params)
{
  Status status;

  status = OpenNI::initialize();
  CHECK_OPENNI_CALL(status, "Unable to initialize OpenNI");

  // Get the first device found
  /** \todo Select a specific device from available ones */
  status = m_device.open(ANY_DEVICE);
  CHECK_OPENNI_CALL(status, "Cannot find a device");

  // Get the depthmap and color streams
  assert(m_device.hasSensor(SENSOR_DEPTH) && "No depth sensor found on current device\n");
  assert(m_device.hasSensor(SENSOR_COLOR) && "No color sensor found on current device\n");

  status = m_depthStream.create(m_device, SENSOR_DEPTH);
  CHECK_OPENNI_CALL(status, "Can't create depth stream\n");
  status = m_colorStream.create(m_device, SENSOR_COLOR);
  CHECK_OPENNI_CALL(status, "Can't create color stream\n");

  // Setup the device and the streams
  status = m_device.setDepthColorSyncEnabled(true);
  CHECK_OPENNI_CALL(status, "Can't enable depth/color synchronization");
  if (params.regDepthOverColor)
  {
    assert(m_device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR) &&
	   "Registration not supported");
    
    status = m_device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
    CHECK_OPENNI_CALL(status, "Can't enable registration");
    
    m_isRegistered = true;
  }
  else if (params.regColorOverDepth)
  {
    CHECK_OPENNI_CALL(!STATUS_OK, "Color over Depth registration not supported");
  }
  else
  {
    status = m_device.setImageRegistrationMode(IMAGE_REGISTRATION_OFF);
    m_isRegistered = false;
  }
  
  const SensorInfo *depthInfo, *colorInfo;
  depthInfo = m_device.getSensorInfo(SENSOR_DEPTH);
  colorInfo = m_device.getSensorInfo(SENSOR_COLOR);
  
  const Array<VideoMode> &depthVideoModes = depthInfo->getSupportedVideoModes();
  bool depthModeFound=false;

  for (int i=0; i<depthVideoModes.getSize(); i++)
  {
    const VideoMode &currMode = depthVideoModes[i];

    if (currMode.getResolutionX()==params.depthWidth &&
	currMode.getResolutionY()==params.depthHeight &&
	currMode.getFps()==params.depthFPS &&
	currMode.getPixelFormat()==PIXEL_FORMAT_DEPTH_1_MM)
    {
      m_depthStream.setVideoMode(currMode);
      m_depthVMode = currMode;
      depthModeFound=true;
      break;
    }
  }

  assert(depthModeFound && "Unable to set selected depth width/height/fps");

  const Array<VideoMode> &colorVideoModes = colorInfo->getSupportedVideoModes();
  bool colorModeFound=false;

  for (int i=0; i<colorVideoModes.getSize(); i++)
  {
    const VideoMode &currMode = colorVideoModes[i];

    if (currMode.getResolutionX()==params.colorWidth &&
	currMode.getResolutionY()==params.colorHeight &&
	currMode.getFps()==params.colorFPS &&
	currMode.getPixelFormat()==PIXEL_FORMAT_RGB888)
    {
      m_colorStream.setVideoMode(currMode);
      m_colorVMode = currMode;
      colorModeFound=true;
      break;
    }
  }

  assert(colorModeFound && "Unable to set selected color width/height/fps");

  // Disable mirroring if requested
  if (!params.mirroring)
  {
    status = m_depthStream.setMirroringEnabled(false);
    CHECK_OPENNI_CALL(status, "Unable to set mirroring on depth stream");
    status = m_colorStream.setMirroringEnabled(false);
    CHECK_OPENNI_CALL(status, "Unable to set mirroring on color stream");
  }  

  // Done with setup, start depth and color nodes
  m_depthStream.start();
  m_colorStream.start();

  // Start acquisition loop thread
  pthread_mutex_init(&m_depthMtx, NULL);
  pthread_mutex_init(&m_colorMtx, NULL);
  pthread_mutex_init(&m_runningMtx, NULL);

  m_isRunning = true;
  m_depthPtr = NULL;
  m_colorPtr = NULL;
  m_depthTimestamp = 0;
  m_colorTimestamp = 0;
  pthread_create(&m_loopThread, NULL, _acquisitionLoop, (void*)this);

  /** \todo Find a better way to wait for acquisition loop start */
  sleep(2);
}


unsigned int OpenNI2Grabber::getDepthWidth()
{
  return m_depthVMode.getResolutionX();
}

unsigned int OpenNI2Grabber::getDepthHeight()
{
  return m_depthVMode.getResolutionY();
}

unsigned int OpenNI2Grabber::getColorWidth()
{
  return m_colorVMode.getResolutionX();
}

unsigned int OpenNI2Grabber::getColorHeight()
{
  return m_colorVMode.getResolutionY();
}


float OpenNI2Grabber::getDepthHFOV()
{
  return (m_isRegistered) ? 
    m_colorStream.getHorizontalFieldOfView()*180.f/M_PI :
    m_depthStream.getHorizontalFieldOfView()*180.f/M_PI;
}

float OpenNI2Grabber::getDepthVFOV()
{
  return (m_isRegistered) ? 
    m_colorStream.getVerticalFieldOfView()*180.f/M_PI :
    m_depthStream.getVerticalFieldOfView()*180.f/M_PI;  
}

float OpenNI2Grabber::getColorHFOV()
{
  return m_colorStream.getHorizontalFieldOfView()*180.f/M_PI;
}

float OpenNI2Grabber::getColorVFOV()
{
  return m_colorStream.getVerticalFieldOfView()*180.f/M_PI;  
}



const unsigned short *OpenNI2Grabber::getDepth(unsigned long *timestamp)
{
  if (timestamp)
  {
    pthread_mutex_lock(&m_depthMtx);
    *timestamp = m_depthTimestamp;
    pthread_mutex_unlock(&m_depthMtx);
  }
  return m_depthPtr;
}

const unsigned char *OpenNI2Grabber::getColor(unsigned long *timestamp)
{
  if (timestamp)
  {
    pthread_mutex_lock(&m_colorMtx);
    *timestamp = m_colorTimestamp;
    pthread_mutex_unlock(&m_colorMtx);
  }
  return m_colorPtr;
}



void OpenNI2Grabber::copyDepth(unsigned short *depth, unsigned long *timestamp)
{
  pthread_mutex_lock(&m_depthMtx);
  std::copy(m_depthPtr, m_depthPtr+getDepthWidth()*getDepthHeight(),
	    depth);
  if (timestamp) *timestamp = m_depthTimestamp;
  pthread_mutex_unlock(&m_depthMtx);
}

void OpenNI2Grabber::copyColor(unsigned char *color, unsigned long *timestamp)
{
  pthread_mutex_lock(&m_colorMtx);
  std::copy(m_colorPtr, m_colorPtr+getColorWidth()*getColorHeight()*3,
	    color);
  if (timestamp) *timestamp = m_colorTimestamp;
  pthread_mutex_unlock(&m_colorMtx);
}



OpenNI2Grabber::~OpenNI2Grabber()
{
  pthread_mutex_lock(&m_runningMtx);
  m_isRunning = false;
  pthread_mutex_unlock(&m_runningMtx);

  pthread_join(m_loopThread, NULL);
  pthread_mutex_destroy(&m_runningMtx);
  pthread_mutex_destroy(&m_colorMtx);
  pthread_mutex_destroy(&m_depthMtx);

  m_colorStream.stop();
  m_depthStream.stop();
  m_colorStream.destroy();
  m_depthStream.destroy();
  m_device.close();
  OpenNI::shutdown();
}


void* _acquisitionLoop(void *_self)
{
  OpenNI2Grabber *self = (OpenNI2Grabber*)_self;
  self->_runLoop();
  return NULL;
}


void OpenNI2Grabber::_runLoop()
{
  VideoStream *streams[] = {&m_depthStream, &m_colorStream};
  int streamIdx;
  
  VideoFrameRef depthFrame, colorFrame;

  while (true)
  {
    pthread_mutex_lock(&m_runningMtx);

    if (!m_isRunning) break;

    OpenNI::waitForAnyStream(streams, 2, &streamIdx, TIMEOUT_FOREVER);
    
    if (!streamIdx)
    {
      pthread_mutex_lock(&m_depthMtx);
      m_depthStream.readFrame(&depthFrame);
      m_depthPtr = (unsigned short*)depthFrame.getData();
      m_depthTimestamp = depthFrame.getTimestamp();
      pthread_mutex_unlock(&m_depthMtx);
    }
    else
    {
      pthread_mutex_lock(&m_colorMtx);
      m_colorStream.readFrame(&colorFrame);
      m_colorPtr = (unsigned char*)colorFrame.getData();
      m_colorTimestamp = colorFrame.getTimestamp();
      pthread_mutex_unlock(&m_colorMtx);
    }
    pthread_mutex_unlock(&m_runningMtx);
  }

  pthread_exit(NULL);
}
