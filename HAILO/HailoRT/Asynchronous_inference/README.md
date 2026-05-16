# Asynchronous Inference using Hailo + OpenCV + GStreamer

This example performs object detection using a USB camera connected to a Hailo device. It receives a `.hef` and images/video/camera as input, and streams the processed video over UDP to another system.

## Features

- Reads frames from image/video/camera sources
- Runs preprocessing, inference, and post-processing in separate threads
- Runs asynchronous inference using a YOLO `.hef` model
- Draws bounding boxes and labels on detected objects
- Streams the processed video over UDP to another system