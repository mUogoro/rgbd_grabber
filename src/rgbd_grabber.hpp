#ifndef __RGBD_GRABBER_HPP
#define __RGBD_GRABBER_HPP

struct RGBDGrabberParams
{
  unsigned int devID;
  unsigned int depthWidth;
  unsigned int depthHeight;
  unsigned int depthFPS;
  unsigned int colorWidth;
  unsigned int colorHeight;
  unsigned int colorFPS;
  bool nearMode;
  bool regDepthOverColor;
  bool regColorOverDepth;
  bool mirroring;
};

class RGBDGrabber
{
  //private:
  //RGBDGrabber(RGBDGrabberParams &params){};
public:
  virtual ~RGBDGrabber(){};

  virtual unsigned int getDepthWidth()=0;
  virtual unsigned int getDepthHeight()=0;
  virtual unsigned int getColorWidth()=0;
  virtual unsigned int getColorHeight()=0;

  virtual float getDepthHFOV()=0;
  virtual float getDepthVFOV()=0;
  virtual float getColorHFOV()=0;
  virtual float getColorVFOV()=0;

  virtual const unsigned short* getDepth(unsigned long *timestamp=NULL)=0;
  virtual const unsigned char* getColor(unsigned long *timestamp=NULL)=0;

  virtual void copyDepth(unsigned short *depth, unsigned long *timestamp=NULL)=0;
  virtual void copyColor(unsigned char *color, unsigned long *timestamp=NULL)=0;
  /*
  virtual void copyDepthAndColor(unsigned short *depth, unsigned char *color,
				 unsigned long *depthTimestamp=NULL,
				 unsigned long *colorTimestamp=NULL)=0;
  */
};


#endif // __RGBD_GRABBER_HPP
