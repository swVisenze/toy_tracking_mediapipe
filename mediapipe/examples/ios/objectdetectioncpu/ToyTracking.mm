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
#include "ToyTracking.h"

int image_width;
int image_height;
bool is_graph_running = false;
size_t cur_time_us = 1;
long obj_id = -1;

std::unique_ptr<mediapipe::CalculatorGraph> graph;

void native_toy_tracking_init(int width, int height) {
    image_width = width;
    image_height = height;
    LOG(INFO) <<"ToyTracking, toy_tracking_init: width: " << image_width << " height: "<< image_height;
//    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSBundle* bundle = [NSBundle bundleWithIdentifier:@"com.visenze.tracking.demo"];
    NSURL* graphURL = [bundle URLForResource:@"mobile_toy_cpu" withExtension:@"binarypb"];
    NSError* configLoadError = nil;
    NSData* data = [NSData dataWithContentsOfURL:graphURL options:0 error:&configLoadError];
    if (!data) {
        NSLog(@"Failed to load MediaPipe graph config: %@", configLoadError);
        return;
    }

    // Parse the graph config resource into mediapipe::CalculatorGraphConfig proto object.
    mediapipe::CalculatorGraphConfig config;
    if(!config.ParseFromArray(data.bytes, data.length)) {
        LOG(INFO) <<"graph config parse failed!!!";
    } else {
        graph = absl::make_unique<mediapipe::CalculatorGraph>();
        absl::Status status = graph->Initialize(config);
        LOG(INFO) <<"graph initialized: "<< status.ok();
    }


}

void native_toy_tracking_reset(const char* code) {

    LOG(INFO) <<"ToyTracking: toy_tracking_reset ";
}

void native_toy_tracking_destroy() {
    LOG(INFO) <<"ToyTracking: toy_tracking_destroy ";
}

const char* native_toy_tracking_tracking(const char* image, int size, int width, int height) {
    LOG(INFO) <<"ToyTracking: toy_tracking_destroy ";
}
