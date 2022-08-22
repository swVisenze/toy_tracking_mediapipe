export GLOG_logtostderr=1

#bazel build --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop:demo_process_image_main
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hello_swtry/hello_swtry \
#   --calculator_graph_config_file=mediapipe/examples/desktop/hello_swtry/test_graph.pbtxt


##### object detection
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_live.pbtxt \
#  --input_video_path=test_video/origin_video.mp4 --output_video_path=test_video/origin_detection_out.mp4

# bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu
# GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_desktop_cpu.pbtxt \
#  --input_video_path=test_video/2_vd217_bg_toys.MOV --output_video_path=test_video/mobileone_v1_dv4_av3_2d_xnnpack.mp4

bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:toy_tracking_cpu
GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/toy_tracking_cpu \
  --calculator_graph_config_file=mediapipe/graphs/tracking/toy_detection_tracking_desktop_live.pbtxt \
 --input_video_path=test_video/2_vd217_bg_toys.MOV \
 --output_video_path=test_video/mobileone_v1_dv4_av3_2d_xnnpack_220822_3

# #bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_image
# GLOG_logptostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_image \
#   --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_live.pbtxt \
#   --input_image_path=test_video/toy_img_2.png --output_image_path=test_video/toy_img_2_output.jpg
#
#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:objectdetectioncpu
#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu
#bazel build -c opt --config=android_arm64 --verbose_failures --sandbox_debug --action_env PYTHON_BIN_PATH=$(which python) mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:helloworld

#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/graphs/object_detection:desktop_tflite_calculators
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

#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:libmediapipe_jni.so
#bazel build -c dbg --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu


#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu
#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:objectdetectioncpu
#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectiongpu:objectdetectiongpu

##### object tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:object_tracking_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/object_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/object_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=test_video/cup_mouse_cellphone2.mp4 --output_video_path=test_video/cup_mouse_cellphone2_out.mp4

#### toy tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:toy_tracking_cpu
#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/toytrackingcpu:toytrackingcpu
# test
#bazel test -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/calculators/tflite:tflite_converter_calculator_test

## see log
# adb logcat -s native | grep graph
# adb shell "export GLOG_logtostderr=1"
## build toy tracking lib
