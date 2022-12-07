export GLOG_logtostderr=1
## see log
# adb logcat -s native | grep graph
# adb shell "export GLOG_logtostderr=1"
## build toy tracking lib
bazel build -c opt --strip=ALWAYS \
     --host_crosstool_top=@bazel_tools//tools/cpp:toolchain \
     --fat_apk_cpu=arm64-v8a,armeabi-v7a \
     --legacy_whole_archive=0 \
     --features=-legacy_whole_archive \
     --copt=-fvisibility=hidden \
     --copt=-ffunction-sections \
     --copt=-fdata-sections \
     --copt=-fstack-protector \
     --copt=-Oz \
     --copt=-fomit-frame-pointer \
     --copt=-DABSL_MIN_LOG_LEVEL=2 \
     --linkopt=-Wl,--gc-sections,--strip-all \
     //mediapipe/java/com/visenze:mediapipe_toy_tracking.aar
