//
// Created by li shiwei on 20/5/22.
//

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"


ABSL_FLAG(std::string, calculator_graph_config_file, "",
"Name of file containing text format CalculatorGraphConfig proto.");

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
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller output1Poller,
            graph.AddOutputStreamPoller("output1"));

    ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller output2Poller,
            graph.AddOutputStreamPoller("output2"));

    MP_RETURN_IF_ERROR(graph.StartRun({}));
    LOG(INFO) << "Start running the calculator graph.";
    for (int i = 0; i < 20; ++i) {
        // send i in input1 every i
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
                "input1", mediapipe::MakePacket<std::string>("input1: " + std::to_string(i)).At(mediapipe::Timestamp(i))));

        if(i % 3 == 2) {
            MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
                    "input2", mediapipe::MakePacket<std::string>("input2: " + std::to_string(i)).At(mediapipe::Timestamp(i))));
        }

        if(i% 2 == 1) {
            MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
                    "input_tick", mediapipe::MakePacket<std::string>("input_tick: " + std::to_string(i)).At(mediapipe::Timestamp(i))));
        }
    }

    // Close the input stream "input_tick".
    MP_RETURN_IF_ERROR(graph.CloseInputStream("input_tick"));
    MP_RETURN_IF_ERROR(graph.CloseInputStream("input1"));
    MP_RETURN_IF_ERROR(graph.CloseInputStream("input2"));

    mediapipe::Packet packet_out1;
    mediapipe::Packet packet_out2;
    while (output1Poller.Next(&packet_out1)) {
        output2Poller.Next(&packet_out2);
        LOG(INFO) << packet_out1.Get<std::string>() <<" at: " << packet_out1.Timestamp();
        LOG(INFO) << packet_out2.Get<std::string>() <<" at: " << packet_out2.Timestamp();
    }
    return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    absl::ParseCommandLine(argc, argv);
    absl::Status run_status = RunMPPGraph();
    if (!run_status.ok()) {
        LOG(ERROR) << "Failed to run the graph: " << run_status.message();
        return EXIT_FAILURE;
    } else {
        LOG(INFO) << "Success!";
    }
    return EXIT_SUCCESS;
}