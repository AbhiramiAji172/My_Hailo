\# Integrating MediaLibrary with HailoRT



This project integrates MediaLibrary with HailoRT for real-time object detection and video streaming.



The pipeline uses GStreamer components including `gsthailovision`, `gsthailoencoder`, and a custom detection stage implemented using HailoRT. Video frames are processed through a custom inference pipeline, overlaid with detection results, encoded, and streamed over UDP.



\## Pipeline Flow



```text

&#x20;                   gsthailovision

&#x20;                          |

&#x20;       ---------------------------------------

&#x20;       |                                     |

&#x20;     sink0                                 sink1

&#x20;       |                                     |

&#x20;     appsink                          gsthailoencoder

&#x20;       |                                     |

&#x20;  GstSourceStage                             |

&#x20;       |                                     |

&#x20; MyDetectionStage                            |

&#x20;(HailoRT Inference)                          |

&#x20;       |                                     |

&#x20;   OverlayStage                              |

&#x20;       |                                     |

&#x20; gsthailoencoder                             |

&#x20;       |                                     |

&#x20;     UDP sink                           UDP sink

```



\## Components



\- \*\*gsthailovision\*\* – Video input and pipeline management

\- \*\*GstSourceStage\*\* – Receives frames from the GStreamer pipeline

\- \*\*MyDetectionStage\*\* – Custom HailoRT-based inference stage

\- \*\*OverlayStage\*\* – Draws detection results on frames

\- \*\*gsthailoencoder\*\* – Encodes processed frames

\- \*\*UDP sink\*\* – Streams output video to a remote receiver



\## Configuration Changes



To match the YOLOv8 model input resolution, the AI processing stream (`sink0`) was resized to \*\*640×640\*\* by modifying:



```bash

/etc/imaging/cfg/hailo15h/imx334/4k/profiles/daylight/ai\_example\_daylight/application\_settings.json

```



\### Modified `sink0` Resolution



```json

{

&#x20; "framerate": 30,

&#x20; "height": 640,

&#x20; "pool\_max\_buffers": 30,

&#x20; "stream\_id": "sink0",

&#x20; "width": 640,

&#x20; "scaling\_mode": "LETTERBOX\_MIDDLE"

}

```



\### Important Changes



\- \*\*height:\*\* `640`

\- \*\*width:\*\* `640`

\- \*\*stream\_id:\*\* `sink0`

\- \*\*scaling\_mode:\*\* `LETTERBOX\_MIDDLE`

