#ifndef __SOFTKINETIC_GRABBER_HPP
#define __SOFTKINETIC_GRABBER_HPP

#include <pthread.h>
#include <semaphore.h>
#include <DepthSense.hxx>
#include <rgbd_grabber.hpp>


using namespace DepthSense;


class SoftkineticGrabber: public RGBDGrabber
{
 private:
  unsigned int depthWidth;
  unsigned int depthHeight;
  unsigned int colorWidth;
  unsigned int colorHeight;
  unsigned int depthFPS;
  unsigned int colorFPS;
  bool nearMode;

  float depthHFOV;
  float depthVFOV;
  float colorHFOV;
  float colorVFOV;

  const unsigned short *depthPtr;
  const unsigned char *colorPtr;

  unsigned long depthTimestamp;
  unsigned long colorTimestamp;

  Context context;
  DepthNode depthNode;
  ColorNode colorNode;
  pthread_t loopThread;
  sem_t loopSem;
  pthread_mutex_t depthMtx;
  pthread_mutex_t colorMtx;
 public:
  SoftkineticGrabber(RGBDGrabberParams &params);
  ~SoftkineticGrabber();

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

  void _copyInternalDepthBuffer(const unsigned short*, unsigned long timestamp);
  void _copyInternalColorBuffer(const unsigned char*, unsigned long timestamp);
  void _runContext();
};

#endif // __SOFTKINETIC_GRABBER_HPP
