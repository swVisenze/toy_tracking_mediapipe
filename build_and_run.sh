export GLOG_logtostderr=1

#bazel build --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/hello_swtry:hello_swtry
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hello_swtry/hello_swtry \
#   --calculator_graph_config_file=mediapipe/examples/desktop/hello_swtry/test_graph.pbtxt


##### object detection
bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu
GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
 --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_desktop_cpu.pbtxt \
 --input_video_path=test_video/2_vd217_bg_toys.MOV --output_video_path=test_video/mobilenetv3_large_2d_075_v1_tf_small_vd217_bg_toys_xnnpack_2.mp4

# bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_image
# GLOG_logptostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_image \
#   --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_live.pbtxt \
#   --input_image_path=test_video/120.jpg --output_image_path=test_video/toy_img_2_output_120.jpg

#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:objectdetectioncpu
#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu
#bazel build -c opt --config=android_arm64 --verbose_failures --sandbox_debug --action_env PYTHON_BIN_PATH=$(which python) mediapipe/examples/android/src/java/com/google/mediapipe/apps/basic:helloworld

##### object tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:object_tracking_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/object_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/object_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=test_video/cup_mouse_cellphone2.mp4 --output_video_path=test_video/cup_mouse_cellphone2_out.mp4

#### toy tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:toy_tracking_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/toy_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/toy_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=test_video/toy_video_2.mp4 --output_video_path=test_video/toy_video_2_tracking_out.mp4

#bazel build -c opt --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/toytrackingcpu:toytrackingcpu
# test
#bazel test -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/calculators/tflite:tflite_converter_calculator_test
