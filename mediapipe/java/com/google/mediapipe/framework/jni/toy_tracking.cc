#include<stdio.h>
#include <iostream>
//#include<string.h>
//#include <cstring>
#include "mediapipe/java/com/google/mediapipe/framework/jni/toy_tracking.h"
#include "mediapipe/java/com/google/mediapipe/framework/jni/jni_util.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/util/android/asset_manager_util.h"
#include "mediapipe/framework/formats/image.h"
#include "mediapipe/framework/formats/image_format.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/port/logging.h"
#include "json.hpp"
#ifdef __ANDROID__
#include "mediapipe/util/android/file/base/helpers.h"
#else
#include "mediapipe/framework/port/file_helpers.h"
#endif  // __ANDROID__

const std::string BINARY_GRAPH_NAME = "toy_mobile_cpu.binarypb";
const std::string INPUT_VIDEO_STREAM_NAME = "input_video_cpu";
const std::string OUTPUT_TRACKED_DETECTION_STREAM_NAME = "tracked_detections";

const std::string STATUS_INIT ="initializing";
const std::string STATUS_TRACKING = "tracking";
const std::string STATUS_LOST = "lost";
const size_t time_gap_us = 50000; // 50 ms -> 20 fps
using JSON = nlohmann::json;

std::string MPG_code = "";
int image_width;
int image_height;
bool is_graph_running = false;
size_t cur_time_us = 1;
long obj_id = -1;

mediapipe::CalculatorGraphConfig graph_config;
std::unique_ptr<mediapipe::CalculatorGraph> graph;
std::unique_ptr<mediapipe::OutputStreamPoller> poller;

JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeInit)(
        JNIEnv* env, jobject thiz, jint jwidth, jint jheight) {
    image_width = jwidth;
    image_height = jheight;
//    jbyte* data_ptr = env->GetByteArrayElements(data, nullptr);
//    int size = env->GetArrayLength(data);
//    absl::Status status =
//            graph->LoadBinaryGraph(reinterpret_cast<char*>(data_ptr), size);
//    env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
    mediapipe::AssetManager* asset_manager =
            Singleton<mediapipe::AssetManager>::get();
    absl::StatusOr<std::string> result = asset_manager->CachedFileFromAsset(BINARY_GRAPH_NAME);
    LOG(INFO) <<"get cached file: "<< result.ok();
    std::string graph_path = result.value();

    std::string graph_config_string;
    absl::Status status =
            mediapipe::file::GetContents(graph_path, &graph_config_string);
    LOG(INFO) << "nativeInit load binary graph at: "<<graph_path << "is ok ?: " <<status.ok();
    if (!graph_config.ParseFromArray(graph_config_string.c_str(),
                                     graph_config_string.length())) {
        LOG(INFO) <<"graph config parse failed!!!";
    } else {
        graph.reset(new mediapipe::CalculatorGraph());
        graph->Initialize(graph_config);
        LOG(INFO) <<"graph initialized";
//        ASSIGN_OR_RETURN(poller,
//                graph.AddOutputStreamPoller(OUTPUT_TRACKED_DETECTION_STREAM_NAME));
//        LOG(INFO) <<"graph AddOutputStreamPoller";
        absl::StatusOr<mediapipe::OutputStreamPoller> result = graph->AddOutputStreamPoller(OUTPUT_TRACKED_DETECTION_STREAM_NAME);
        if(result.ok()) {
            poller = std::make_unique<mediapipe::OutputStreamPoller>(std::move(result.value()));
            LOG(INFO) <<"add poller";
        }
    }
}

JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeDestroy)(
        JNIEnv* env, jobject thiz) {
    MPG_code = "";
    image_width = 0;
    image_height = 0;
    is_graph_running = false;
    obj_id = -1;
    cur_time_us = 1;
    graph->CloseAllInputStreams();
    graph->WaitUntilDone();
    graph->Cancel();
    graph.reset(nullptr);
    poller.reset(nullptr);
}

// this is native method NOT JNI method
JNIEXPORT void toy_tracking_reset(const char* code) {
    MPG_code = code;
    if(is_graph_running) {
        absl::Status status = graph->CloseAllInputStreams();
        if(status.ok()) {
            LOG(INFO) << "CloseAllInputStreams";
            graph->WaitUntilDone();
            LOG(INFO) << "graph done";
            graph->Cancel();
        }
    }
    obj_id = -1;
    cur_time_us = 1;
    absl::Status status = graph->StartRun({});
    is_graph_running = status.ok();
    LOG(INFO) << "StartRunningGraph running: " << is_graph_running;
}

// this is native method NOT JNI method
JNIEXPORT const char* toy_tracking_tracking(const char* image_buffer, int size, int width, int height) {
//    //buffer is  in RGBA image format
    if(!is_graph_running) {
        LOG(INFO) << "graph not run yet";
        return "";
    }

//    LOG(INFO) << "width: " << width << " height: "<< height << " size: " << size;
    const int expected_buffer_size = width * height * 3;
    if (size != expected_buffer_size) {
        LOG(INFO) << "unmatched size, expected size: " << expected_buffer_size;
        return "";
    }
    // create image frame, format RGBA
    auto image_format = mediapipe::ImageFormat::SRGB;
    auto image_frame = std::make_unique<mediapipe::ImageFrame>(
            image_format, width, height,
            mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
    std::memcpy(image_frame->MutablePixelData(), image_buffer,
                image_frame->PixelDataSize());
    // create input packet
//    size_t frame_timestamp_us =
//            (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    cur_time_us = cur_time_us + time_gap_us;
    mediapipe::Packet packet = mediapipe::Adopt(image_frame.release()).At(mediapipe::Timestamp(cur_time_us));
//    LOG(INFO) << "input frame_timestamp_us: " << cur_time_us;
    // send packet to graph
    graph->AddPacketToInputStream(INPUT_VIDEO_STREAM_NAME, packet);

    mediapipe::Packet output_packet;
    if (!poller->Next(&output_packet)) {
        LOG(INFO) << "cannot get output packet";
        return "";
    }
//    LOG(INFO) << "output packet: "<<output_packet.DebugString();
    auto& out_detections = output_packet.Get<std::vector<mediapipe::Detection>>();

    std::string status;
    std::string track_box;
    std::string debug_message;
    if(out_detections.size()>0) {
        status = STATUS_TRACKING;
        auto detection = out_detections[0];
        auto& relative_bbox = detection.location_data().relative_bounding_box();
        float min_x = relative_bbox.xmin();
        float min_y = relative_bbox.ymin();
//        LOG(INFO) <<"min_x: "<< min_x << " min_y: "<<min_y;
        float max_x = min_x + relative_bbox.width();
        float max_y = min_y + relative_bbox.height();
//        LOG(INFO) <<"max_x: "<< max_x << " max_y: "<<max_y;

        track_box = std::to_string(min_x) + "," + std::to_string(min_y) + ";" + std::to_string(max_x) + "," + std::to_string(max_y);
        debug_message = detection.track_id();
        obj_id = detection.detection_id();
    } else {
        if(obj_id > 0) {
            status = STATUS_LOST;
        } else {
            status = STATUS_INIT;
        }
        track_box = "";
        debug_message = "NO DETECTION OUTPUT";
    }
//    LOG(INFO) <<"obj_id: "<< obj_id;
    auto jsonObj = JSON::object();
    jsonObj["status"] = status;
    jsonObj["track_box"] = track_box;
    jsonObj["debug_message"] = debug_message;
    std::string jsonStr = jsonObj.dump();
//    LOG(INFO) <<"json output: "<< jsonStr;
    // so strange this can output to unity ...
    std::string dump = "" + jsonStr;
    const char *output = dump.c_str();
//    const char *output = MPG_code.c_str();
    return output;
 }

//JNIEXPORT void toy_tracking_init() {
//    LOG(INFO) << "init function called";
//    image_width = 640;
//    image_height = 480;
//}

//JNIEXPORT void toy_tracking_destroy() {
//    MPG_code = "";
//    image_width = 0;
//    image_height = 0;
//}