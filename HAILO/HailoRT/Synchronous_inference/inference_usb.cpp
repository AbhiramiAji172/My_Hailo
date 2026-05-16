#include "hailo/hailort.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#if defined(__unix__)
#include <sys/mman.h>
#endif

#define BATCH_SIZE 1
#define CAMERA_INDEX "/dev/video3"
#define STREAM_HOST "192.168.1.49"   
#define STREAM_PORT 5000

using namespace hailort;

// COCO class labels
std::vector<std::string> coco_classes = {
    "person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light",
    "fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow",
    "elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee",
    "skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard",
    "tennis racket","bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple",
    "sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair","couch",
    "potted plant","bed","dining table","toilet","tv","laptop","mouse","remote","keyboard",
    "cell phone","microwave","oven","toaster","sink","refrigerator","book","clock","vase",
    "scissors","teddy bear","hair drier","toothbrush"
};

static std::shared_ptr<uint8_t> page_aligned_alloc(size_t size)
{
    auto addr = mmap(NULL, size, PROT_WRITE | PROT_READ,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (MAP_FAILED == addr)
        throw std::bad_alloc();

    return std::shared_ptr<uint8_t>(
        reinterpret_cast<uint8_t*>(addr),
        [size](void *addr) { munmap(addr, size); });
}

int main()
{
    try
    {
        // Camera Initialization
        cv::VideoCapture cap(CAMERA_INDEX);
        if (!cap.isOpened()) {
            std::cerr << "Failed to open camera\n";
            return -1;
        }

        //cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        //cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

        int stream_w = 640;
        int stream_h = 640;
        int fps = 30;

        // Gstreamer pipeline for sending post processed output
        std::string pipeline =
            "appsrc ! videoconvert ! "
            "video/x-raw,format=I420 ! "
            "x264enc tune=zerolatency bitrate=800 speed-preset=ultrafast ! "
            "rtph264pay config-interval=1 pt=96 ! "
            "udpsink host=" STREAM_HOST " port=5000";

        cv::VideoWriter writer(
            pipeline,
            cv::CAP_GSTREAMER,
            0,
            fps,
            cv::Size(stream_w, stream_h),
            true
        );

        if (!writer.isOpened()) {
            std::cerr << "Failed to open GStreamer writer\n";
            return -1;
        }

        // Hailo Initializations
        // Create Virtual Hailo Device
        auto vdevice = VDevice::create().expect("Failed to create VDevice");
        // Load .hef file
        auto infer_model = vdevice->create_infer_model("/home/root/abhirami/model_downloaded/yolov8s.hef").expect("Failed to create infer model");

        int model_h = 640;
        int model_w = 640;

        infer_model->set_batch_size(BATCH_SIZE);

        // Configure inference model
        auto configured_infer_model = infer_model->configure().expect("Failed configure");
        // Create input and output bindings
        auto bindings = configured_infer_model.create_bindings().expect("Failed bindings");

        std::vector<std::shared_ptr<uint8_t>> buffer_guards;
        std::vector<DmaMappedBuffer> buffer_map_guards;

        // Creating input buffer and binding
        std::shared_ptr<uint8_t> input_buffer;
        for (const auto &name : infer_model->get_input_names()) {
            size_t sz = infer_model->input(name)->get_frame_size();
            input_buffer = page_aligned_alloc(sz);
            buffer_guards.push_back(input_buffer);

            auto map = DmaMappedBuffer::create(
                *vdevice, input_buffer.get(), sz,
                HAILO_DMA_BUFFER_DIRECTION_H2D).expect("Failed map input");

            buffer_map_guards.push_back(std::move(map));
            bindings.input(name)->set_buffer(MemoryView(input_buffer.get(), sz));
        }

        // Creating output buffer and binding
        std::shared_ptr<uint8_t> output_buffer;
        for (const auto &name : infer_model->get_output_names()) {
            size_t sz = infer_model->output(name)->get_frame_size();
            output_buffer = page_aligned_alloc(sz);
            buffer_guards.push_back(output_buffer);

            auto map = DmaMappedBuffer::create(
                *vdevice, output_buffer.get(), sz,
                HAILO_DMA_BUFFER_DIRECTION_D2H).expect("Failed map output");

            buffer_map_guards.push_back(std::move(map));
            bindings.output(name)->set_buffer(MemoryView(output_buffer.get(), sz));
        }

        std::cout << "Setup done\n";

        while (true)
        {
            auto t1 = std::chrono::high_resolution_clock::now();

            // Frame Capture
            cv::Mat frame;
            cap >> frame;
            if (frame.empty())
                continue;

            // Preprocess
            cv::Mat rgb;
            cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB); 
            cv::resize(rgb, rgb, cv::Size(model_w, model_h));

            if (!rgb.isContinuous())
                rgb = rgb.clone();

            // Copying pre-processed frame into input buffer
            std::memcpy(input_buffer.get(), rgb.data, infer_model->input()->get_frame_size());

            // Inference
            auto job = configured_infer_model.run_async(
                bindings,
                [](const AsyncInferCompletionInfo&) {}
            ).expect("Failed run inference");

            job.wait(std::chrono::milliseconds(1000));

            // Postprocess
            uint8_t *data = output_buffer.get();
            int num_classes = 80;
            size_t offset = 0;

            // Parse detection class wise
            for (int class_id = 0; class_id < num_classes; class_id++)
            {
                float count_f = *(float*)(data + offset);
                int count = static_cast<int>(count_f); 
                offset += sizeof(float);

                for (int i = 0; i < count; i++)
                {
                    auto bbox = *(hailo_bbox_float32_t*)(data + offset);

                    if (bbox.score >= 0.5)
                    {
                        int x1 = static_cast<int>(bbox.x_min * rgb.cols);
                        int y1 = static_cast<int>(bbox.y_min * rgb.rows);
                        int x2 = static_cast<int>(bbox.x_max * rgb.cols);
                        int y2 = static_cast<int>(bbox.y_max * rgb.rows);

                        // Drawing bounding box
                        cv::rectangle(rgb,
                                      cv::Point(x1, y1),
                                      cv::Point(x2, y2),
                                      cv::Scalar(0, 255, 0), 2);  

                        std::string label =
                            coco_classes[class_id] + " " +
                            cv::format("%.2f", bbox.score);

                        // Putting text
                        cv::putText(rgb,
                                    label,
                                    cv::Point(x1, y1 - 5),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.5,
                                    cv::Scalar(0, 255, 0),
                                    2);
                    }

                    offset += sizeof(hailo_bbox_float32_t);
                }
            }

            // Streaming
            cv::Mat stream_frame;
            cv::cvtColor(rgb, stream_frame, cv::COLOR_RGB2BGR);
            cv::resize(stream_frame, stream_frame, cv::Size(stream_w, stream_h));
            writer.write(stream_frame);

            // FPS (total processing FPS) Calculation
            auto t2 = std::chrono::high_resolution_clock::now();
            double fps_now = 1.0 / std::chrono::duration<double>(t2 - t1).count();
            std::cout << "FPS: " << fps_now << std::endl;
        }
    }
    catch (const hailort_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
