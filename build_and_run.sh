export GLOG_logtostderr=1

#bazel build --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/hello_swtry:hello_swtry
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hello_swtry/hello_swtry \
#   --calculator_graph_config_file=mediapipe/examples/desktop/hello_swtry/test_graph.pbtxt


##### object detection
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_live.pbtxt \
#  --input_video_path=test_video/origin_video.mp4 --output_video_path=test_video/origin_detection_out.mp4

#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:objectdetectioncpu
#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectiongpu:objectdetectiongpu

### toy detection
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_desktop_cpu.pbtxt \
#  --input_video_path=../mediapipe_test_video/vd113_bg_toys.MOV --output_video_path=../mediapipe_test_video/vd113_bg_toys_detection_3.mp4

#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_image
#GLOG_logptostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_image \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_toy_live.pbtxt \
#  --input_image_path=../mediapipe_test_video/test_img.png --output_image_path=../mediapipe_test_video/test_img_out.jpg

#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu
#bazel build -c dbg --config=android_arm64 mediapipe/examples/android/src/java/com/google/mediapipe/apps/objectdetectioncpu:toydetectioncpu


##### object tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:object_tracking_cpu

#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/object_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/object_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=test_video/cup_mouse_cellphone2.mp4 --output_video_path=test_video/cup_mouse_cellphone2_out.mp4

#### toy tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:toy_tracking_cpu
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/toy_tracking_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/tracking/toy_detection_tracking_desktop_live.pbtxt \
#  --input_video_path=../mediapipe_test_video/vd113_bg_toys.MOV --output_video_path=../mediapipe_test_video/vd113_bg_toys_tracking_detection_3.mp4

#bazel build -c opt --config=android_arm64 --define MEDIAPIPE_PROFILING=1 mediapipe/examples/android/src/java/com/google/mediapipe/apps/toytrackingcpu:toytrackingcpu
# test
#bazel test -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/calculators/tflite:tflite_converter_calculator_test