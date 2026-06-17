\# Integrating MediaLibrary with HailoRT



This project integrates MediaLibrary with HailoRT for real-time object detection and video streaming.



The pipeline uses GStreamer components including `gsthailovision`, `gsthailoencoder`, and a custom detection stage implemented using HailoRT. Video frames are processed through a custom inference pipeline, overlaid with detection results, encoded, and streamed over UDP.



\## Pipeline Flow



```text

&#x20;                   gsthailovision

&#x20;                          |

&#x20;       ---------------------------------------

&#x20;       |                                     |

&#x20;       | sink0                               | sink1

&#x20;       |                                     |

&#x20;       v                                     v

&#x20;     appsink                         gsthailoencoder

&#x20;       |                                     |

&#x20;       |                                     v

&#x20;       |                                 UDP sink

&#x20;       |

&#x20;       v

&#x20;  GstSourceStage

&#x20;       |

&#x20;       v

&#x20; MyDetectionStage

&#x20;(HailoRT Inference)

&#x20;       |

&#x20;       v

&#x20;   OverlayStage

&#x20;       |

&#x20;       v

&#x20;     gsthailoencoder

&#x20;       |

&#x20;       v

&#x20;     UDP sink

```



\## Components



\- \*\*gsthailovision\*\* – Video input and pipeline management

\- \*\*GstSourceStage\*\* – Receives frames from the GStreamer pipeline

\- \*\*MyDetectionStage\*\* – Custom HailoRT-based inference stage

\- \*\*OverlayStage\*\* – Draws detection results on frames

\- \*\*gsthailoencoder\*\* – Encodes processed frames

\- \*\*UDP sink\*\* – Streams output video to a remote receiver



```

\## Configuration Changes



To match the YOLOv8 model input resolution, the AI input stream (`sink0`) was resized to \*\*640×640\*\* by modifying:



```bash

/etc/imaging/cfg/hailo15h/imx334/4k/profiles/daylight/ai\_example\_daylight/application\_settings.json

```



\### Modified `application\_settings.json` 



```json

{

&#x20; "version": "3.0.0",

&#x20; "metadata": {

&#x20;   "architecture": "hailo15h",

&#x20;   "content\_hash": "5f9a58ac1deaaaccba7e72e3f8c3822a9590238ae5f97e5a75e7fb24ef3985de",

&#x20;   "description": "",

&#x20;   "generation\_timestamp": "2026-02-24T14:33:08.219775+00:00"

&#x20; },

&#x20; "application\_input\_streams": {

&#x20;   "format": "IMAGE\_FORMAT\_NV12",

&#x20;   "method": "INTERPOLATION\_TYPE\_BILINEAR",

&#x20;   "resolutions": \[

&#x20;     {

&#x20;       "framerate": 30,

&#x20;       "height": 640,

&#x20;       "pool\_max\_buffers": 30,

&#x20;       "stream\_id": "sink0",

&#x20;       "width": 640,

&#x20;       "scaling\_mode": "LETTERBOX\_MIDDLE"

&#x20;     },

&#x20;     {

&#x20;       "framerate": 30,

&#x20;       "height": 720,

&#x20;       "pool\_max\_buffers": 30,

&#x20;       "stream\_id": "sink1",

&#x20;       "width": 1280

&#x20;     },

&#x20;     {

&#x20;       "framerate": 15,

&#x20;       "height": 1080,

&#x20;       "pool\_max\_buffers": 15,

&#x20;       "stream\_id": "sink2",

&#x20;       "width": 1920

&#x20;     }

&#x20;   ]

&#x20; },

&#x20; "digital\_zoom": {

&#x20;   "enabled": false,

&#x20;   "magnification": 1.0,

&#x20;   "mode": "DIGITAL\_ZOOM\_MODE\_ROI",

&#x20;   "roi": {

&#x20;     "height": 1800,

&#x20;     "width": 2800,

&#x20;     "x": 200,

&#x20;     "y": 200

&#x20;   }

&#x20; },

&#x20; "flip": {

&#x20;   "direction": "FLIP\_DIRECTION\_HORIZONTAL",

&#x20;   "enabled": false

&#x20; },

&#x20; "gmv": {

&#x20;   "frequency": 0.0,

&#x20;   "source": "isp"

&#x20; },

&#x20; "hailort": {

&#x20;   "device-id": "device0"

&#x20; },

&#x20; "motion\_detection": {

&#x20;   "buffer\_pool\_size": 8,

&#x20;   "enabled": false,

&#x20;   "resolution": {

&#x20;     "framerate": 30,

&#x20;     "height": 480,

&#x20;     "width": 640

&#x20;   },

&#x20;   "roi": {

&#x20;     "height": 480,

&#x20;     "width": 640,

&#x20;     "x": 0,

&#x20;     "y": 0

&#x20;   },

&#x20;   "sensitivity\_level": "MEDIUM",

&#x20;   "threshold": 0.5

&#x20; },

&#x20; "optical\_zoom": {

&#x20;   "enabled": true,

&#x20;   "magnification": 1.0

&#x20; },

&#x20; "rotation": {

&#x20;   "angle": "ROTATION\_ANGLE\_180",

&#x20;   "enabled": false

&#x20; }

}

```



\### Important Changes



\- \*\*height:\*\* `640`

\- \*\*width:\*\* `640`

\- \*\*stream\_id:\*\* `sink0`

\- \*\*scaling\_mode:\*\* `LETTERBOX\_MIDDLE`



