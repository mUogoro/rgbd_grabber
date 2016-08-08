# RGBDGrabber Library

The RGBDGrabber library provides a unified and simplified interface for accessing the color and depth streams of various consumer RGD-Depth cameras.

The library has been developed by the [Engineering for Health and Wellbeing group](http://www.ehw.ieiit.cnr.it/?q=computervision) at the [Institute of Electronics, Computer and Telecommunication Engineering](http://www.ieiit.cnr.it)
of the National Research Council of Italy (CNR).

## Supported cameras
The library supports the following cameras:
- Primesense Carmine and Microsoft Kinect V1 through the OpenNI2 SDK;
- SoftKinetic DS525 and Creative Senz3D throgh the SoftKinetic DepthSense SDK;

## Supported platforms
The library supports both Windows and Linux operating systems. More in detail, it has been tested on the following platforms:
- Ubuntu Linux 14.04
- Microsoft Windows 8 (Visual Studio 2013)

Only 64bit platforms are supported (porting to 32bit should be trivial).

## Supported features
|             | OpenNI2       | DepthSense  |
|-----------|---------------|------------|
| Depth modes | 640x480x30fps 320x240x30fps| 320x240x25/30/50/60fps|
| Color modes | 640x480x30fps 320x240x30fps| 320x240x25/30fps 640x480x25/30fps |
| Multiple device support | No | No |
| Color over depth registration | No | No |
| Depth over color registration | Yes | No |
| Mirroring | No | No |
| Color mode | RGB | BGR |


## Dependencies
The following dependencies are needed to compile the library:
- [CMake](https://cmake.org/download/)
- GNU Gcc (Linux) or Microsoft Visual Studio (Windows)
- [OpenNI2 SDK](http://structure.io/openni)
- [SoftKinetic DepthSense SDK](http://www.softkinetic.com/support/download)(registration required for Linux installation files)
- [OpenCV 2.4](http://opencv.org/downloads.html)
- [pthreads-win32](http://www.sourceware.org/pthreads-win32)(Windows only)

## Installation
```
git clone https://github.com/mUogoro/rgbd_grabber.git rgbd_grabber
```
On Linux:
```
cd rgbd_grabber
mkdir build && cd build
cmake ..
make && make install
```
On Windows:
```
cd rgbd_grabber
mkdir build
cd build
cmake ..
```
Go to the build directory, open the Visual Studio solution file and compile the library. Compile the INSTALL target to perform installation.

## Using the library
Within the test directory some examples are provided. Look at the inlined documentation for details about how to use the library.

## Final notes
The library is still on development. Further supported devices and features will be added in the near future.