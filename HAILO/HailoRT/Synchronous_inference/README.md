# Synchronous Inference using Hailo + OpenCV + GStreamer

Real-time object detection using a USB camera connected to a Hailo AI accelerator.  
The application captures frames, runs synchronous YOLO inference on the Hailo device, draws bounding boxes, and streams the processed video over UDP to a receiver system.

---

## Features

- Captures frames from a USB camera (V4L2)
- Runs synchronous inference using a YOLO `.hef` model on Hailo hardware
- Draws bounding boxes and class labels on detected objects
- Streams processed video over UDP using RTP/H.264 via GStreamer
- Displays real-time FPS on the output frame

---

## Build

```bash
cmake -S . -B build \
  -DHailoRT_DIR=/media/drive_b/sysroots/armv8a-poky-linux/usr/lib/cmake/HailoRT

cmake --build build
```

> **Note:** Adjust `-DHailoRT_DIR` to match the actual HailoRT installation path on your system.

---

## Run

```bash
./infer_usb 
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
