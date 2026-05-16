# Asynchronous Inference using Hailo + OpenCV + GStreamer

This example performs object detection using a USB camera connected to a Hailo device. It receives a `.hef` and images/video/camera as input, and streams the processed video over UDP to another system.

---

## Features

- Reads frames from image/video/camera sources
- Runs preprocessing, inference, and post-processing in separate threads
- Runs asynchronous inference using a YOLO `.hef` model
- Draws bounding boxes and labels on detected objects
- Streams the processed video over UDP to another system
---

## Build

```bash
cmake -S . -B build \
  -DHailoRT_DIR=/media/drive_b/sysroots/armv8a-poky-linux/usr/lib/cmake/HailoRT

cmake --build build
```

> **Note:** Adjust `-DHailoRT_DIR` to match the actual HailoRT installation path on your system.

---

## Run on HAILO board

```bash
./infer_app --net <hef_path> --input <image_or_video_or_camera_path>
```
---

## Receiving the Stream

Run the following GStreamer pipeline on the **receiver machine** to display the video:

```bash
gst-launch-1.0 -v \
  udpsrc port=5000 \
    caps="application/x-rtp,media=video,encoding-name=H264,payload=96" \
  ! rtph264depay \
  ! avdec_h264 \
  ! videoconvert \
  ! autovideosink sync=false
```
