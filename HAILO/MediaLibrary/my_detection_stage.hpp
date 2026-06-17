#include "hailo/hailort.hpp"
#include "hailo_analytics/pipeline/core/stage.hpp"
#include "media_library/media_library_buffer.hpp" 
#include "hailo_analytics/pipeline/core/buffer.hpp"
#include <opencv2/opencv.hpp>
#include "hailo_postprocess_tools/objects/hailo_objects.hpp"
#include "hailo_postprocess_tools/image_utils/overlay_native.hpp"


#include <sys/mman.h>

using namespace hailort;

static const std::vector<std::string> coco_classes = {
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

class MyDetectionStage : public hailo_analytics::pipeline::ThreadedStage
{
public:
    MyDetectionStage(std::string name, size_t queue_size = 4)
        : hailo_analytics::pipeline::ThreadedStage(name, queue_size, false)
    {}

    hailo_analytics::pipeline::AppStatus init() override
    {
        m_vdevice = VDevice::create().expect("VDevice fail");

        m_infer_model = m_vdevice->create_infer_model(
            "/home/root/abhirami/yolov8s.hef")
            .expect("HEF fail");

        m_infer_model->set_batch_size(1);
        m_configured = m_infer_model->configure().expect("Configure fail");
        m_bindings = m_configured.create_bindings().expect("Bindings fail");

        // Input buffer
        for (const auto &name : m_infer_model->get_input_names()) {
            size_t sz = m_infer_model->input(name)->get_frame_size();
            m_input_buf = page_aligned_alloc(sz);
            m_input_size = sz;
            auto map = DmaMappedBuffer::create(
                *m_vdevice, m_input_buf.get(), sz,
                HAILO_DMA_BUFFER_DIRECTION_H2D).expect("Map input fail");
            m_dma_maps.push_back(std::move(map));
            m_bindings.input(name)->set_buffer(MemoryView(m_input_buf.get(), sz));
        }

        // Output buffer
        for (const auto &name : m_infer_model->get_output_names()) {
            size_t sz = m_infer_model->output(name)->get_frame_size();
            m_output_buf = page_aligned_alloc(sz);
            auto map = DmaMappedBuffer::create(
                *m_vdevice, m_output_buf.get(), sz,
                HAILO_DMA_BUFFER_DIRECTION_D2H).expect("Map output fail");
            m_dma_maps.push_back(std::move(map));
            m_bindings.output(name)->set_buffer(MemoryView(m_output_buf.get(), sz));
        }

        return hailo_analytics::pipeline::AppStatus::SUCCESS;
    }

    hailo_analytics::pipeline::AppStatus process(
        hailo_analytics::pipeline::BufferPtr buffer) override
    {
        HailoMediaLibraryBufferPtr in_buffer = buffer->get_buffer();
	int in_width = in_buffer->buffer_data->width;
	int in_height = in_buffer->buffer_data->height;
	//std::cout<<"INPUT : " << in_width << " " << in_height << " "<< std::endl;

       std::memcpy(m_input_buf.get(), in_buffer->get_plane_ptr(0), m_input_size);

        /*--- INFERENCE ---*/
        auto job = m_configured.run_async(
            m_bindings,
            [](const AsyncInferCompletionInfo&) {}
        ).expect("Inference fail");
        job.wait(std::chrono::milliseconds(1000));

        /*--- POSTPROCESS ---*/
        auto roi = buffer->get_roi();
        uint8_t *data = m_output_buf.get();
        size_t offset = 0;

        for (int class_id = 0; class_id < 80; class_id++) {
            int count = static_cast<int>(*(float*)(data + offset));
            offset += sizeof(float);

            for (int i = 0; i < count; i++) {
                auto bbox = *(hailo_bbox_float32_t*)(data + offset);
                offset += sizeof(hailo_bbox_float32_t);

                if (bbox.score < 0.5f) continue;

                roi->add_object(std::make_shared<HailoDetection>(
                    HailoBBox(
                        bbox.x_min,
                        bbox.y_min,
                        bbox.x_max - bbox.x_min,  
                        bbox.y_max - bbox.y_min 
                    ),
                    coco_classes[class_id],
                    bbox.score
                ));
		std::cout<< "class: " << coco_classes[class_id] << std::endl;

            }
        }

        send_to_subscribers(buffer);
        return hailo_analytics::pipeline::AppStatus::SUCCESS;
    }

    hailo_analytics::pipeline::AppStatus deinit() override
    {
        m_dma_maps.clear();
        return hailo_analytics::pipeline::AppStatus::SUCCESS;
    }

private:
    std::unique_ptr<VDevice> m_vdevice;
    std::shared_ptr<InferModel> m_infer_model;
    ConfiguredInferModel m_configured;
    ConfiguredInferModel::Bindings m_bindings;
    std::shared_ptr<uint8_t> m_input_buf;
    std::shared_ptr<uint8_t> m_output_buf;
    size_t m_input_size = 0;
    std::vector<DmaMappedBuffer> m_dma_maps;
};
