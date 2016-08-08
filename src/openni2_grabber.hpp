#ifndef __OPENNI2_GRABBER_HPP
#define __OPENNI2_GRABBER_HPP

#include <pthread.h>
#include <semaphore.h>
#include <OpenNI.h>
#include <rgbd_grabber.hpp>


class OpenNI2Grabber: public RGBDGrabber
{
private:
  openni::Device m_device;
  openni::VideoStream m_depthStream;
  openni::VideoStream m_colorStream;
  openni::VideoMode m_depthVMode;
  openni::VideoMode m_colorVMode;

  const unsigned short *m_depthPtr;
  const unsigned char *m_colorPtr;
  
  unsigned long m_depthTimestamp;
  unsigned long m_colorTimestamp;

  sem_t m_loopSem;
  pthread_t m_loopThread;
  pthread_mutex_t m_depthMtx;
  pthread_mutex_t m_colorMtx;
  pthread_mutex_t m_runningMtx;
  bool m_isRunning;
  bool m_isRegistered;

public:
  OpenNI2Grabber(RGBDGrabberParams &params);
  ~OpenNI2Grabber();

  unsigned int getDepthWidth();
  unsigned int getDepthHeight();
  unsigned int getColorWidth();
  unsigned int getColorHeight();

  float getDepthHFOV();
  float getDepthVFOV();
  float getColorHFOV();
  float getColorVFOV();


  const unsigned short* getDepth(unsigned long *timestamp=NULL);
  const unsigned char* getColor(unsigned long *timestamp=NULL);

  void copyDepth(unsigned short *depth, unsigned long *timestamp=NULL);
  void copyColor(unsigned char *color, unsigned long *timestamp=NULL);

  void _runLoop();
};


#endif // __OPENNI2_GRABBER_HPP
