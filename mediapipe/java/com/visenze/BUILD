load("//mediapipe/java/com/google/mediapipe:mediapipe_aar.bzl", "mediapipe_aar")

mediapipe_aar(
    name = "mediapipe_toy_tracking",
    calculators = [
        "//mediapipe/graphs/tracking:toy_mobile_calculators"
    ],
    srcs = [
        "ToyTracking.java"
    ],
    assets = [
        "//mediapipe/graphs/tracking:toy_mobile_cpu_binary_graph",
        "//mediapipe/models:toy_detection_mobilenetv3_large075_2d_v3_portrait_v3_d23_a5_e17.tflite",
        "//mediapipe/models:object_detection_labelmap.txt"
    ],
#    deps = [
#        "@maven//:com_google_flogger_flogger",
#        "@maven//:com_google_flogger_flogger_system_backend",
#        "@maven//:com_google_guava_guava",
#    ]

)
