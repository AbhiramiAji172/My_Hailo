# Integrating MediaLibrary with HailoRT

This project integrates MediaLibrary with HailoRT for real-time object detection and video streaming.

The pipeline uses GStreamer components including `gsthailovision`, `gsthailoencoder`, and a custom detection stage implemented using HailoRT. Video frames are processed through a custom inference pipeline, overlaid with detection results, encoded, and streamed over UDP.

## Pipeline Flow

```text
                    gsthailovision
                           |
        ---------------------------------------
        |                                     |
      sink0                                 sink1
        |                                     |
      appsink                          gsthailoencoder
        |                                     |
   GstSourceStage                             |
        |                                     |
  MyDetectionStage                            |
 (HailoRT Inference)                          |
        |                                     |
    OverlayStage                              |
        |                                     |
  gsthailoencoder                             |
        |                                     |
      UDP sink                           UDP sink
```

## Components

- **gsthailovision** – Video input and pipeline management
- **GstSourceStage** – Receives frames from the GStreamer pipeline
- **MyDetectionStage** – Custom HailoRT-based inference stage
- **OverlayStage** – Draws detection results on frames
- **gsthailoencoder** – Encodes processed frames
- **UDP sink** – Streams output video to a remote receiver

## Configuration Changes

To match the YOLOv8 model input resolution, the AI processing stream (`sink0`) was resized to **640×640** by modifying:

```bash
/etc/imaging/cfg/hailo15h/imx334/4k/profiles/daylight/ai_example_daylight/application_settings.json
```

### Modified `sink0` Resolution

```json
{
  "framerate": 30,
  "height": 640,
  "pool_max_buffers": 30,
  "stream_id": "sink0",
  "width": 640,
  "scaling_mode": "LETTERBOX_MIDDLE"
}
```

### Important Changes

- **height:** `640`
- **width:** `640`
- **stream_id:** `sink0`
- **scaling_mode:** `LETTERBOX_MIDDLE`