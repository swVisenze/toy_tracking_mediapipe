#include<stdio.h>
#include <iostream>
//#include<string.h>
//#include <cstring>
#include "mediapipe/java/com/google/mediapipe/framework/jni/toy_tracking.h"
#include "mediapipe/framework/port/logging.h"

std::string MPG_code;
int image_width;
int image_height;

JNIEXPORT void toy_tracking_init() {
    LOG(INFO) << "init function called";
    image_width = 640;
    image_height = 480;
}

JNIEXPORT void toy_tracking_reset(const char* code) {
    MPG_code = code;
    LOG(INFO) << "reset function with code: "<<MPG_code;
}

JNIEXPORT const char* toy_tracking_tracking() {
    LOG(INFO) << "tracking function return code: "<< MPG_code;
    const char *output = MPG_code.c_str();
    return output;
//    strcpy(result, MPG_code.c_str());
 }

JNIEXPORT void toy_tracking_destroy() {
    MPG_code = "";
    image_width = 0;
    image_height = 0;
}