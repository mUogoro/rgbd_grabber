#ifndef __LIBFREENECT2_GRABBER_HPP
#define __LIBFREENECT2_GRABBER_HPP

#include <string>
#include <pthread.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/registration.h>
#include <rgbd_grabber.hpp>


class Libfreenect2GrabberListener;

class Libfreenect2Grabber: public RGBDGrabber
{
private:
  libfreenect2::Freenect2 m_context;
  libfreenect2::PacketPipeline *m_pipeline;
  libfreenect2::Freenect2Device *m_dev;

  float m_depthHFOV;
  float m_depthVFOV;
  float m_colorHFOV;
  float m_colorVFOV;

  libfreenect2::Frame *m_depthPtr;
  libfreenect2::Frame *m_colorPtr;
  libfreenect2::Frame *m_undistortedPtr;
  libfreenect2::Frame *m_registeredPtr;
  libfreenect2::Registration *m_reg;
  
  unsigned long m_depthTimestamp;
  unsigned long m_colorTimestamp;

  pthread_mutex_t m_depthMtx;
  pthread_mutex_t m_colorMtx;
  bool m_isRegistered;
  bool m_isRunning;

  Libfreenect2GrabberListener *m_listener;


public:
  Libfreenect2Grabber(RGBDGrabberParams &params);
  ~Libfreenect2Grabber();

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
  void copyDepthAndColor(unsigned short *depth, unsigned char *color,
			 unsigned long *depthTimestamp=NULL,
			 unsigned long *colorTimestamp=NULL);

  void _copyDepthPtr(libfreenect2::Frame *frame);
  void _copyColorPtr(libfreenect2::Frame *frame);
};

#endif // __LIBFREENECT2_GRABBER_HPP
