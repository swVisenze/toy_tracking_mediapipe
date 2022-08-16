# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load(
    "@build_bazel_rules_apple//apple:ios.bzl",
    "ios_application",
    "ios_framework"
)
load(
    "//mediapipe/examples/ios:bundle_id.bzl",
    "BUNDLE_ID_PREFIX",
    "example_provisioning",
)

licenses(["notice"])

MIN_IOS_VERSION = "11.0"

alias(
    name = "objectdetectioncpu",
    actual = "ObjectDetectionCpuApp",
)

ios_application(
    name = "ObjectDetectionCpuApp",
    app_icons = ["//mediapipe/examples/ios/common:AppIcon"],
#    bundle_id = BUNDLE_ID_PREFIX + ".ObjectDetectionCpu",
    bundle_id="com.visenze.tracking.demo",
    families = [
        "iphone",
        "ipad",
    ],
    infoplists = [
        "//mediapipe/examples/ios/common:Info.plist",
        "Info.plist",
    ],
    minimum_os_version = MIN_IOS_VERSION,
#    provisioning_profile = example_provisioning(),
    provisioning_profile = "toy_tracking.mobileprovision",
    deps = [
#        ":ObjectDetectionCpuAppLibrary",
        ":ToyDetectionCpuAppLibrary",
        "@ios_opencv//:OpencvFramework",
    ],
)

objc_library(
    name = "ObjectDetectionCpuAppLibrary",
    data = [
        "//mediapipe/graphs/object_detection:mobile_cpu_binary_graph",
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
    ],
    deps = [
        "//mediapipe/examples/ios/common:CommonMediaPipeAppLibrary",
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/graphs/object_detection:mobile_calculators",
        ],
    }),
)


objc_library(
    name = "ToyDetectionCpuAppLibrary",
    data = [
        "//mediapipe/graphs/object_detection:mobile_toy_cpu_binary_graph",
#        "//mediapipe/models:toy_detection_mobilenet.tflite",
        "//mediapipe/models:toy_detection_m3l075_2d_v4_dv4_av3.tflite",
        "//mediapipe/models:object_detection_labelmap.txt",
    ],
    deps = [
        "//mediapipe/examples/ios/common:CommonMediaPipeAppLibrary",
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/graphs/object_detection:mobile_calculators",
        ],
    }),
)

ios_framework(
    name = "ToyTrackingFramework",
    hdrs = [
        "UnityBridge.h"
    ],
    bundle_id="com.visenze.tracking.demo",
    provisioning_profile = "toy_tracking.mobileprovision",
    bundle_name = "ToyTracking",
    families = ["iphone", "ipad"],
    infoplists = [
        "Info.plist",
    ],
    minimum_os_version = MIN_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [
        ":ToyTrackingCpuAppLibrary",
        "@ios_opencv//:OpencvFramework",
    ],
)


objc_library(
    name = "ToyTrackingCpuAppLibrary",
    srcs = [
      "ToyTracking.mm",
      "UnityBridge.mm"
    ],
    hdrs = [
        "ToyTracking.h",
        "UnityBridge.h"
    ],
    data = [
        "//mediapipe/graphs/tracking:toy_mobile_cpu_binary_graph",
        "//mediapipe/models:toy_detection_m3l075_2d_v4_dv4_av3.tflite",
        "//mediapipe/models:object_detection_labelmap.txt",
    ],
    sdk_frameworks = [
        "AVFoundation",
        "CoreGraphics",
        "CoreMedia",
        "UIKit"
    ],
    alwayslink = True,
    deps = [
#        "//mediapipe/examples/ios/common:CommonMediaPipeAppLibrary",
        "//mediapipe/objc:mediapipe_framework_ios",
#        "//mediapipe/objc:mediapipe_input_sources_ios",
#        "//mediapipe/objc:mediapipe_layer_renderer",
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/graphs/tracking:toy_mobile_calculators"
        ],
    }),
)
