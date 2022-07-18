#include<stdio.h>
#include <iostream>
//#include<string.h>
//#include <cstring>
#include "mediapipe/java/com/google/mediapipe/framework/jni/toy_tracking.h"
#include "mediapipe/java/com/google/mediapipe/framework/jni/graph.h"
#include "mediapipe/java/com/google/mediapipe/framework/jni/jni_util.h"
#include "mediapipe/util/android/asset_manager_util.h"
#include "mediapipe/framework/port/logging.h"

const std::string BINARY_GRAPH_NAME = "toy_mobile_cpu.binarypb";
const std::string INPUT_VIDEO_STREAM_NAME = "input_video_cpu";
const std::string OUTPUT_TRACKED_DETECTION_STREAM_NAME = "tracked_detections";

std::string MPG_code;
int image_width;
int image_height;

mediapipe::android::Graph* graph;


JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeInit)(
        JNIEnv* env, jobject thiz, jint jwidth, jint jheight) {
    image_width = jwidth;
    image_height = jheight;
    graph = new mediapipe::android::Graph();
//    jbyte* data_ptr = env->GetByteArrayElements(data, nullptr);
//    int size = env->GetArrayLength(data);
//    absl::Status status =
//            graph->LoadBinaryGraph(reinterpret_cast<char*>(data_ptr), size);
//    env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
//    const char* path_ref = env->GetStringUTFChars(path, nullptr);
//    // Make a copy of the string and release the jni reference.
//    std::string path_to_graph(path_ref);
//    env->ReleaseStringUTFChars(path, path_ref);

//    auto graph_path = Singleton<AssetManager>::get()->CachedFileFromAsset(BINARY_GRAPH_NAME);
    mediapipe::AssetManager* asset_manager =
            Singleton<mediapipe::AssetManager>::get();
    absl::StatusOr<std::string> result = asset_manager->CachedFileFromAsset(BINARY_GRAPH_NAME);
    LOG(INFO) <<"get cached file: "<< result.ok();
    std::string graph_path = result.value();
    absl::Status status =
            graph->LoadBinaryGraph(graph_path);
    LOG(INFO) << "nativeInit load binary graph at: "<<graph_path << "is ok ?: " <<status.ok();
    mediapipe::android::ThrowIfError(env, status);
}

JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeDestroy)(
        JNIEnv* env, jobject thiz) {
    MPG_code = "";
    image_width = 0;
    image_height = 0;
    delete graph;
}

// this is native method NOT JNI method
JNIEXPORT void toy_tracking_reset(const char* code) {
    MPG_code = code;
    LOG(INFO) << "reset function with code: "<<MPG_code;
}

// this is native method NOT JNI method
JNIEXPORT const char* toy_tracking_tracking() {
//    LOG(INFO) << "tracking function return code: "<< MPG_code << " image width: " << image_width << " image height: " << image_height;
    const char *output = MPG_code.c_str();
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