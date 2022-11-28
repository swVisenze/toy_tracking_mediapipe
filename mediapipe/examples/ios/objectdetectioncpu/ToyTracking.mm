//
//  ToyTracking.m
//  ObjectDetectionCpuApp
//
//  Created by li shiwei on 23/7/22.
//

#import <Foundation/Foundation.h>
#include<stdio.h>
#include <iostream>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image.h"
#include "mediapipe/framework/formats/image_format.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/objc/json.hpp"
#include "ToyTracking.h"

@implementation ToyTracking : NSObject


//const std::string BINARY_GRAPH_NAME = "toy_mobile_cpu";
const std::string INPUT_VIDEO_STREAM_NAME = "input_video_cpu";
const std::string OUTPUT_TRACKED_DETECTION_STREAM_NAME = "tracked_detections";

const std::string STATUS_INIT ="initializing";
const std::string STATUS_TRACKING = "tracking";
const std::string STATUS_SEARCHING = "searching";
const std::string STATUS_LOST = "lost";
const size_t time_gap_us = 50000; // 50 ms -> 20 fps
using JSON = nlohmann::json;

int buffer_frames=20;

int lost_frames = 0;
int image_width;
int image_height;
volatile bool is_graph_running = false;
size_t cur_time_us = 1;
long obj_id = -1;

mediapipe::CalculatorGraphConfig graph_config;
std::unique_ptr<mediapipe::CalculatorGraph> graph;
std::unique_ptr<mediapipe::OutputStreamPoller> poller;

- (void)loadGraph:(int) width height: (int) height {
    // Temp way to turn on/off loggings from mediapipe
    // set it to 100 to turn off.
    FLAGS_minloglevel = -1;
    image_width = width;
    image_height = height;
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSString* graphName = [bundle objectForInfoDictionaryKey:@"GraphName"];
    NSURL* graphURL = [bundle URLForResource:graphName withExtension:@"binarypb"];
    NSError* configLoadError = nil;
    NSData* data = [NSData dataWithContentsOfURL:graphURL options:0 error:&configLoadError];
    if (!data) {
        NSLog(@"Failed to load MediaPipe graph config: %@", configLoadError);
        const char *errorMessage = [[configLoadError localizedDescription]
                                    cStringUsingEncoding: NSISOLatin1StringEncoding];
        LOG(ERROR) << "Failed to load MediaPipe graph config" << errorMessage;
        return;
    }

    // Parse the graph config resource into mediapipe::CalculatorGraphConfig proto object.
//    mediapipe::CalculatorGraphConfig config;
    if(!graph_config.ParseFromArray(data.bytes, data.length)) {
        LOG(ERROR) <<"graph config parse failed!!!";
        return;
    }
}

- (void)destroy {
    LOG(INFO) << "TrackingiOSSDK: toy_tracking_destroy ";
    image_width = 0;
    image_height = 0;
    obj_id = -1;
    cur_time_us = 1;
    lost_frames = 0;
    buffer_frames = 20;

    if (is_graph_running) {
        LOG(INFO) << "TrackingiOSSDK: closing Graph: close all input streams";
        graph->CloseAllInputStreams();
        // graph->WaitUntilDone();
        LOG(INFO) << "TrackingiOSSDK: closing Graphs: abort the graph";
        graph->Cancel();
    }
    graph.reset(nullptr);
    is_graph_running = false;

    LOG(INFO) << "TrackingiOSSDK: toy_tracking_destroy close poller";
    poller.reset(nullptr);
}

- (bool)reset:(int)frames {
    buffer_frames = frames;
    if(is_graph_running) {
        is_graph_running = false;
        absl::Status status = graph->CloseAllInputStreams();
        graph->Cancel();
        LOG(INFO) << "TrackingiOSSDK: graph cancelled";
        poller.reset(nullptr);
    }
    obj_id = -1;
    cur_time_us = 1;
    lost_frames = 0;
    graph.reset(new mediapipe::CalculatorGraph());
    LOG(INFO) <<"TrackingiOSSDK: graph reset with new object";
    absl::Status status = graph->Initialize(graph_config);
    LOG(INFO) <<"TrackingiOSSDK: graph initialized: "<< status.ok();

    absl::StatusOr<mediapipe::OutputStreamPoller> result = graph->AddOutputStreamPoller(OUTPUT_TRACKED_DETECTION_STREAM_NAME);
    if(result.ok()) {
        poller = std::make_unique<mediapipe::OutputStreamPoller>(std::move(result.value()));
//        poller.reset(std::move(result.value()));
//        LOG(INFO) <<"add poller";
        absl::Status status = graph->StartRun({});
        is_graph_running = status.ok();
        LOG(INFO) << "TrackingiOSSDK: StartRunningGraph running: " << is_graph_running;
        return is_graph_running;
    } else {
        return false;
    }
}

- (NSString*)tracking:(const unsigned char*)image_buffer size:(int) size width:(int)width height:(int)height {
    if(!is_graph_running) {
        LOG(INFO) << "graph not run yet";
        return @"";
    }

    LOG(INFO) << "TrackingiOSSDK: Buffer info: width: " << width << " height: "<< height << " size: " << size;
    const int expected_buffer_size = width * height * 3;
    if (size != expected_buffer_size) {
        LOG(ERROR) << "unmatched size, expected size: " << expected_buffer_size;
        return @"";
    }
    // create image frame, format RGBA
    auto image_format = mediapipe::ImageFormat::SRGB;
    auto image_frame = std::make_unique<mediapipe::ImageFrame>(
            image_format, width, height,
            mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
    std::memcpy(image_frame->MutablePixelData(), image_buffer,
                image_frame->PixelDataSize());

    cur_time_us = cur_time_us + time_gap_us;
    mediapipe::Packet packet = mediapipe::Adopt(image_frame.release()).At(mediapipe::Timestamp(cur_time_us));
    LOG(INFO) << "TrackingiOSSDK: input frame_timestamp_us: " << cur_time_us;
    // send packet to graph
    graph->AddPacketToInputStream(INPUT_VIDEO_STREAM_NAME, packet);

    mediapipe::Packet output_packet;
    if (!poller->Next(&output_packet)) {
        LOG(ERROR) << "TrackingiOSSDK: cannot get output packet";
        return @"";
    }

//    while(poller->Next(&output_packet) && output_packet.Timestamp().Value() < cur_time_us) {
//        LOG(INFO) << "pull latest packet, current time: "<<cur_time_us;
//    }
//    if(output_packet.IsEmpty()) {
//        LOG(INFO) << "no output";
//        return @"";
//    }

    auto& out_detections = output_packet.Get<std::vector<mediapipe::Detection>>();

    std::string status;
    std::string track_box;
    std::string debug_message;
    int detection_index = 0;

    if(out_detections.size()>0) {
        status = STATUS_TRACKING;
        lost_frames = 0;
        obj_id = out_detections[0].detection_id();
        for(int i=1; i < out_detections.size(); i++) {
            if(out_detections[i].detection_id() < obj_id) {
                obj_id = out_detections[i].detection_id();
                detection_index = i;
            }
        }
        auto& detection = out_detections[detection_index];
        LOG(INFO) << "TrackingiOSSDK: number of out detections: "<<out_detections.size();
        for(int i=0; i< out_detections.size(); i++) {
            LOG(INFO) << "TrackingiOSSDK: out detection: " << i
                      << " object id: "<< out_detections[i].detection_id();
        }

        auto& relative_bbox = detection.location_data().relative_bounding_box();
        float min_x = relative_bbox.xmin();
        float min_y = relative_bbox.ymin();
        float max_x = min_x + relative_bbox.width();
        float max_y = min_y + relative_bbox.height();

        track_box = std::to_string(min_x)
                + "," + std::to_string(min_y)
                + ";" + std::to_string(max_x)
                + "," + std::to_string(max_y);
        debug_message = detection.track_id();
    } else {
        if(obj_id > 0) {
            lost_frames += 1;
            if(lost_frames > buffer_frames) {
                status = STATUS_LOST;
            } else {
                status = STATUS_SEARCHING;
            }
        } else {
            status = STATUS_INIT;
        }
        track_box = "";
        debug_message = "NO DETECTION OUTPUT";
    }
    auto jsonObj = JSON::object();
    jsonObj["status"] = status;
    jsonObj["track_box"] = track_box;
    jsonObj["debug_message"] = debug_message;
    std::string jsonStr = jsonObj.dump();
    LOG(INFO) <<"TrackingiOSSDK: json output: "<< jsonStr;

    NSString* result = [NSString stringWithUTF8String:jsonStr.c_str()];
    return result;
}


@end


