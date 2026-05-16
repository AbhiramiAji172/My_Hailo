\# Synchronous Inference using Hailo + OpenCV + GStreamer



This project performs real-time object detection using a USB camera and Hailo accelerator.  

The application captures frames from the camera, runs synchronous inference using a YOLO model (`.hef`), draws detected bounding boxes, and streams the processed video over UDP to a Windows host system.



\---



\## Features



\- Capture live video from USB camera (`/dev/video3`)

\- Run synchronous inference on Hailo accelerator

\- Perform preprocessing using OpenCV

\- Parse object detection output

\- Draw bounding boxes and labels

\- Stream processed output via GStreamer

\- Display real-time processing FPS



\---



\## Project Flow



The application performs the following steps:



1\. Initialize camera input

2\. Load Hailo HEF model

3\. Allocate DMA input/output buffers

4\. Capture frame from camera

5\. Preprocess frame

6\. Run synchronous inference

7\. Postprocess detections

8\. Draw bounding boxes

9\. Stream processed frame to host PC

10\. Print total pipeline FPS



\---



\## Dependencies



Make sure the following libraries are installed:



\- HailoRT

\- OpenCV 4

\- GStreamer

\- CMake

\- pthread



\---



\## Build Instructions



\### Create build directory



```bash

mkdir build

cd build

```



\### Run CMake



```bash

cmake ..

```



\### Build project



```bash

make

```



\---



\## Run



```bash

./synchronous\_inference

```



\---



\## Stream Receiver (Windows)



To view the streamed output on Windows:



```bash

gst-launch-1.0 -v udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H264,payload=96" ! rtph264depay ! avdec\_h264 ! autovideosink sync=false

```



\---



\## Configuration



Update the following macros in source code as needed:



```cpp

\#define CAMERA\_INDEX "/dev/video3"

\#define STREAM\_HOST "192.168.1.49"

\#define STREAM\_PORT 5000

```



\---



\## Notes



\- FPS shown is the total pipeline FPS, including:

&#x20; - frame capture

&#x20; - preprocessing

&#x20; - inference

&#x20; - postprocessing

&#x20; - streaming



\- Actual FPS depends on model complexity and processing time.

\- Streamed output is sent to the configured Windows receiver via UDP.



\---

