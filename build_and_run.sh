export GLOG_logtostderr=1

#bazel build --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop:demo_process_image_main
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hello_swtry/hello_swtry \
#   --calculator_graph_config_file=mediapipe/examples/desktop/hello_swtry/test_graph.pbtxt


### toy detection
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu

#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/desktop/object_detection:object_detection_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_desktop_cpu.pbtxt \
#  --input_video_path=../mediapipe_test_video/test_05_Jul/1_vd217_bg_clear.MOV --output_video_path=../mediapipe_test_video/test_05_Jul/1_vd217_bg_clear_detection_3.mp4

#GLOG_logtostderr=1 ./object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_desktop_cpu.pbtxt \
#  --input_video_path=../mediapipe_test_video/test_05_Jul/1_vd217_bg_clear.MOV --output_video_path=../mediapipe_test_video/test_05_Jul/1_vd217_bg_clear_detection_3.mp4


#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_image
#GLOG_logptostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_image \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_live.pbtxt \
#  --input_image_path=../mediapipe_test_video/test_img.png --output_image_path=../mediapipe_test_video/test_img_out_1_quan.jpg

##### object tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:object_tracking_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/object_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/object_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=test_video/cup_mouse_cellphone2.mp4 --output_video_path=test_video/cup_mouse_cellphone2_out.mp4

#### toy tracking
# bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:toy_tracking_cpu
 GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/toy_tracking_cpu \
   --calculator_graph_config_file=mediapipe/graphs/tracking/toy_detection_tracking_desktop_live.pbtxt \
   --input_video_path=../mediapipe_test_video/test_05_Jul/1_vd217_bg_clear.MOV \
   --output_video_path=../mediapipe_test_video/test_05_Jul/0_retarget/1_vd217_bg_clear

### toy tracking android demo app.
#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/toytrackingcpu:toytrackingcpu


## see log
# adb logcat -s native | grep graph
# adb shell "export GLOG_logtostderr=1"
## build toy tracking lib
# bazel build -c opt --strip=ALWAYS \
#     --host_crosstool_top=@bazel_tools//tools/cpp:toolchain \
#     --fat_apk_cpu=arm64-v8a,armeabi-v7a \
#     --legacy_whole_archive=0 \
#     --features=-legacy_whole_archive \
#     --copt=-fvisibility=hidden \
#     --copt=-ffunction-sections \
#     --copt=-fdata-sections \
#     --copt=-fstack-protector \
#     --copt=-Oz \
#     --copt=-fomit-frame-pointer \
#     --copt=-DABSL_MIN_LOG_LEVEL=2 \
#     --linkopt=-Wl,--gc-sections,--strip-all \
#     //mediapipe/java/com/visenze:mediapipe_toy_tracking.aar

#bazel build -c opt --config=ios_arm64 mediapipe/examples/ios/objectdetectioncpu:ToyTrackingFramework


#bazel build --copt=-fembed-bitcode --apple_bitcode=embedded --config=ios_arm64 mediapipe/examples/ios/objectdetectioncpu:ToyTrackingFramework

