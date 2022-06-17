// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
#include <cstdlib>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include "tensorflow/lite/interpreter.h"



constexpr char kInputStream[] = "input_image";
constexpr char kOutputStream[] = "output_image";
constexpr char kWindowName[] = "MediaPipe";

ABSL_FLAG(std::string, calculator_graph_config_file, "",
"Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_image_path, "",
"Full image path");
ABSL_FLAG(std::string, output_image_path, "",
"Full path of where to save result ");

namespace mediapipe {
    cv::Mat GetRgb(std::string path) {
        cv::Mat bgr = cv::imread(path);
        cv::Mat rgb(bgr.rows, bgr.cols, CV_8UC3);
        int from_to[] = {0, 2, 1, 1, 2, 0};
        cv::mixChannels(&bgr, 1, &rgb, 1, from_to, 3);
        return rgb;
    }

    cv::Mat GetRgba(std::string path) {
        cv::Mat bgr = cv::imread(path);
        cv::Mat rgba(bgr.rows, bgr.cols, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        int from_to[] = {0, 2, 1, 1, 2, 0};
        cv::mixChannels(&bgr, 1, &bgr, 1, from_to, 3);
        return bgr;
    }


    absl::Status RunMPPGraph() {
        std::string calculator_graph_config_contents;
        MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
                absl::GetFlag(FLAGS_calculator_graph_config_file),
                &calculator_graph_config_contents));
        LOG(INFO) << "Get calculator graph config contents: "
                  << calculator_graph_config_contents;
        mediapipe::CalculatorGraphConfig config =
                mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
                        calculator_graph_config_contents);

        LOG(INFO) << "Initialize the calculator graph.";
        mediapipe::CalculatorGraph graph;
        std::vector <Packet> output_packets;
        tool::AddVectorSink("tensor", &config, &output_packets);

        MP_RETURN_IF_ERROR(graph.Initialize(config));
        MP_RETURN_IF_ERROR(graph.StartRun({}));

        LOG(INFO) << "load input image...";
        RET_CHECK(!absl::GetFlag(FLAGS_input_image_path).empty());
        cv::Mat input = GetRgb(absl::GetFlag(FLAGS_input_image_path));

        ImageFrame input_image(
                input.channels() == 4 ? ImageFormat::SRGBA : ImageFormat::SRGB,
                input.cols, input.rows, input.step, input.data, [](uint8 *) {});

        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
                "input_image",
                MakePacket<ImageFrame>(std::move(input_image)).At(Timestamp(0))));

        // Wait until the calculator done processing.
        MP_RETURN_IF_ERROR(graph.WaitUntilIdle());

        const std::vector <TfLiteTensor> &tensor_vec =
                output_packets[0].Get < std::vector < TfLiteTensor >> ();

        LOG(INFO) << "tensor vector size: " << tensor_vec.size();
        for (int i = 0; i < tensor_vec.size(); i++) {
            const TfLiteTensor *tensor = &tensor_vec[i];
            const float *tensor_buffer = tensor->data.f;
            LOG(INFO) << "tensor_buffer " << i << " size: "
                      << sizeof(tensor_buffer);
        }

        MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
        return graph.WaitUntilDone();
    }
}

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    absl::ParseCommandLine(argc, argv);
    absl::Status run_status = mediapipe::RunMPPGraph();
    if (!run_status.ok()) {
        LOG(ERROR) << "Failed to run the graph: " << run_status.message();
        return EXIT_FAILURE;
    } else {
        LOG(INFO) << "Success!";
    }
    return EXIT_SUCCESS;
}