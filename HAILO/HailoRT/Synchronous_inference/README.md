# Synchronous Inference using Hailo + OpenCV + GStreamer

This project performs real-time object detection using a USB camera and Hailo accelerator.

The application captures frames from the camera, runs synchronous inference using a YOLO model (`.hef`), draws detected bounding boxes, and streams the processed video over UDP to a Windows host system.

## Features

- Capture live video from USB camera (`/dev/video3`)
- Run synchronous inference on Hailo accelerator
- Perform preprocessing using OpenCV
- Parse object detection output
- Draw bounding boxes and labels
- Stream processed output via GStreamer
- Display real-time processing FPS

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make