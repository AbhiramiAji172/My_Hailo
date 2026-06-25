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
  Analytics pipeline                          |
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
- **Analytics Pipeline** – tiling + detection + aggregator
- **OverlayStage** – Draws detection results on frames
- **gsthailoencoder** – Encodes processed frames
- **UDP sink** – Streams output video to a remote receiver

