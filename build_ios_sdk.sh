export GLOG_logtostderr=1
bazel build -c opt --config=ios_arm64 mediapipe/examples/ios/objectdetectioncpu:ToyTrackingFramework --verbose_failures
