#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <libfreenect2_grabber.hpp>


#define CHECK_LIBFREENECT2_CALL(__CONDITION, __ERROR_STRING)\
{\
  if (!(__CONDITION))\
  {\
    std::stringstream errss;\
    errss << __ERROR_STRING << std::endl;\
    throw std::runtime_error(errss.str());\
  }\
}


class Libfreenect2GrabberListener: public libfreenect2::FrameListener
{
private:
  Libfreenect2Grabber *m_grabber;
public:
  Libfreenect2GrabberListener(Libfreenect2Grabber &grabber)
  {
    m_grabber = &grabber;
  }

  bool onNewFrame(libfreenect2::Frame::Type type, libfreenect2::Frame *frame)
  {
    bool retVal = false;

    if (type==libfreenect2::Frame::Color)
    {
      m_grabber->_copyColorPtr(frame);
      retVal = true;
    }
    else if (type==libfreenect2::Frame::Depth)
    {
      m_grabber->_copyDepthPtr(frame);
      retVal = true;
    }

    return retVal;
  }
};

Libfreenect2Grabber::Libfreenect2Grabber(RGBDGrabberParams &params)
{
  pthread_mutex_init(&m_depthMtx, NULL);
  pthread_mutex_init(&m_colorMtx, NULL);

  /*
  //m_pipeline = new libfreenect2::CpuPacketPipeline();
  //m_dev = m_context.openDefaultDevice(m_pipeline);
  m_dev = m_context.openDefaultDevice();
  CHECK_LIBFREENECT2_CALL(m_dev, "Unable to open default device");
  */

  int nDevs = m_context.enumerateDevices();
  CHECK_LIBFREENECT2_CALL(nDevs, "No device found");
  m_dev = m_context.openDevice(params.devID);
  CHECK_LIBFREENECT2_CALL(m_dev, "Unable to open selected device");

  m_depthPtr = NULL;
  m_colorPtr = NULL;
  m_depthTimestamp = 0;
  m_colorTimestamp = 0;

  m_listener = new Libfreenect2GrabberListener(*this);
  m_dev->setColorFrameListener(m_listener);
  m_dev->setIrAndDepthFrameListener(m_listener);

  m_isRegistered = true; m_reg = NULL;
  m_dev->start();

  if (params.regColorOverDepth)
  {
    m_reg = new libfreenect2::Registration(m_dev->getIrCameraParams(),
					   m_dev->getColorCameraParams());
    m_undistortedPtr = new libfreenect2::Frame(512, 424, 4);
    m_registeredPtr = new libfreenect2::Frame(512, 424, 4);
  }
  else if (params.regDepthOverColor)
  {
    CHECK_LIBFREENECT2_CALL(false, "Depth over Color registration not yet supported");
  }
  else
  {
    m_isRegistered = false;
  }

  libfreenect2::Freenect2Device::ColorCameraParams colorParams = m_dev->getColorCameraParams();
  libfreenect2::Freenect2Device::IrCameraParams depthParams = m_dev->getIrCameraParams();

  m_depthHFOV = 2.0f*atan(512.f/(2.f*depthParams.fx)) * 180.f/M_PI;
  m_depthVFOV = 2.0f*atan(424.f/(2.f*depthParams.fy)) * 180.f/M_PI;
  m_colorHFOV = 2.0f*atan(1920.f/(2.f*colorParams.fx)) * 180.f/M_PI;
  m_colorVFOV = 2.0f*atan(1080.f/(2.f*colorParams.fy)) * 180.f/M_PI;
}

Libfreenect2Grabber::~Libfreenect2Grabber()
{
  m_dev->stop();
  m_dev->close();

  pthread_mutex_destroy(&m_depthMtx);
  pthread_mutex_destroy(&m_colorMtx);

  if (m_isRegistered)
  {
    delete m_undistortedPtr;
    delete m_registeredPtr;
    delete m_reg;
  }

  delete m_listener;
  delete m_dev;
  //delete m_pipeline;
}

unsigned int Libfreenect2Grabber::getDepthWidth()
{
  return 512;
}

unsigned int Libfreenect2Grabber::getDepthHeight()
{
  return 424;
}

unsigned int Libfreenect2Grabber::getColorWidth()
{
  return m_isRegistered ? 512 : 1920;
}

unsigned int Libfreenect2Grabber::getColorHeight()
{
  return m_isRegistered ? 424 : 1080;
}

float Libfreenect2Grabber::getDepthHFOV()
{
  //return 70.6f;
  return m_depthHFOV;
}

float Libfreenect2Grabber::getDepthVFOV()
{
  //return 60.f;
  return m_depthVFOV;
}

float Libfreenect2Grabber::getColorHFOV()
{
  //return 84.1;
  return m_isRegistered ? m_depthHFOV : m_colorHFOV;
}

float Libfreenect2Grabber::getColorVFOV()
{
  //return 53.8f;
  return m_isRegistered ? m_depthVFOV : m_colorVFOV;
}

const unsigned short* Libfreenect2Grabber::getDepth(unsigned long *timestamp)
{
  //return m_depthPtr;
  return NULL;
}

const unsigned char* Libfreenect2Grabber::getColor(unsigned long *timestamp)
{
  //return m_colorPtr;
  return NULL;
}

void Libfreenect2Grabber::copyDepth(unsigned short *depth, unsigned long *timestamp)
{
  pthread_mutex_lock(&m_depthMtx);

  if (timestamp)
  {
    *timestamp = static_cast<unsigned long>(m_depthPtr->timestamp);
  }

  if (m_depthPtr)
  {
    std::copy((float*)m_depthPtr->data,
	      ((float*)m_depthPtr->data)+getDepthWidth()*getDepthHeight(), depth);
  }

  pthread_mutex_unlock(&m_depthMtx);
}

void Libfreenect2Grabber::copyColor(unsigned char *color, unsigned long *timestamp)
{
  pthread_mutex_lock(&m_colorMtx);

  if (m_colorPtr)
  {
    if (m_isRegistered)
    {
      if (m_reg)
      {
	// Lock the depth internal pointer. It will be used for the color-over-depth
	// registration
	pthread_mutex_lock(&m_depthMtx);
      
	if (m_depthPtr)
        {
	  m_reg->apply(m_colorPtr, m_depthPtr, m_undistortedPtr, m_registeredPtr);

	
	  std::copy((unsigned char*)m_registeredPtr->data,
		    ((unsigned char*)m_registeredPtr->data)+getColorWidth()*getColorHeight()*4, color);
	
	  if (timestamp)  *timestamp =  static_cast<unsigned long>(m_depthPtr->timestamp);
	}

	pthread_mutex_unlock(&m_depthMtx);
      }
    }
    else
    {
      std::copy((unsigned char*)m_colorPtr->data,
		((unsigned char*)m_colorPtr->data)+getColorWidth()*getColorHeight()*4, color);

      if (timestamp)  *timestamp =  static_cast<unsigned long>(m_colorPtr->timestamp);
    }
  }
  
  pthread_mutex_unlock(&m_colorMtx);
}



void Libfreenect2Grabber::_copyDepthPtr(libfreenect2::Frame *frame)
{
  pthread_mutex_lock(&m_depthMtx);
  delete m_depthPtr;
  m_depthPtr = frame;
  pthread_mutex_unlock(&m_depthMtx);
}

void Libfreenect2Grabber::_copyColorPtr(libfreenect2::Frame *frame)
{
  pthread_mutex_lock(&m_colorMtx);
  delete m_colorPtr;
  m_colorPtr = frame;
  pthread_mutex_unlock(&m_colorMtx);
}
