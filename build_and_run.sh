export GLOG_logtostderr=1

#bazel build --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/hello_swtry:hello_swtry
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/hello_swtry/hello_swtry \
#   --calculator_graph_config_file=mediapipe/examples/desktop/hello_swtry/test_graph.pbtxt


##### object detection
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_tflite
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_tflite \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_tflite_graph.pbtxt \
#  --input_side_packets=input_video_path=test_video/origin_video.mp4,output_video_path=test_video/origin_detection_out.mp4

#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_detection:object_detection_cpu
#GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_cpu \
#  --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_live.pbtxt \
#  --input_video_path=test_video/origin_video.mp4 --output_video_path=test_video/origin_detection_out.mp4


##### object tracking
#bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/object_tracking:object_tracking_cpu

GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/object_tracking/object_tracking_cpu \
  --calculator_graph_config_file=mediapipe/graphs/tracking/object_detection_tracking_desktop_live.pbtxt \
  --input_video_path=test_video/cup_mouse_cellphone2.mp4 --output_video_path=test_video/cup_mouse_cellphone2_out.mp4