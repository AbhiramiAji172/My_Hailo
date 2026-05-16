# Synchronous Inference using Hailo + OpenCV + GStreamer

This project performs real-time object detection using a USB camera connected to a Hailo device.

The application captures frames from the camera, runs synchronous inference using a YOLO model (`.hef`), draws detected bounding boxes, and streams the processed video over UDP to another system.

## Features

The application:
- captures frames from a USB camera
- runs synchronous inference using a YOLO `.hef` model
- draws bounding boxes on detected objects
- streams the processed video over UDP to another system
- displays runtime FPS
