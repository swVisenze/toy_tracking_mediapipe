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
#include <iostream>
#include <fstream>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/tool/sink.h"

constexpr char kInputStream[] = "input_video";
constexpr char KOutputDetection[] = "tracked_detections";
constexpr char KOutputDetectionWithId[] = "detections_with_id";
constexpr char kOutputStream[] = "output_video";
//constexpr char kOutputDetectionVideo[] = "detection_output_video";
constexpr char kWindowName[] = "MediaPipe";

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

std::string getBBoxString(mediapipe::Detection detection) {
  auto& relative_bbox = detection.location_data().relative_bounding_box();
  float min_x = relative_bbox.xmin();
  float min_y = relative_bbox.ymin();
  float max_x = min_x + relative_bbox.width();
  float max_y = min_y + relative_bbox.height();

  auto result_str = std::to_string(min_x) + " " + std::to_string(min_y) + ";"
                    + std::to_string(max_x) + " " + std::to_string(max_y);
  return result_str;
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

  std::vector<mediapipe::Packet> tracked_detections_packets;
  mediapipe::tool::AddVectorSink(KOutputDetection, &config, &tracked_detections_packets);

  std::vector<mediapipe::Packet> detections_with_id_packets;
  mediapipe::tool::AddVectorSink(KOutputDetectionWithId, &config, &detections_with_id_packets);

  LOG(INFO) << "Initialize the calculator graph.";
  mediapipe::CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();
  if (load_video) {
    capture.open(absl::GetFlag(FLAGS_input_video_path));
  } else {
    capture.open(0);
  }
  RET_CHECK(capture.isOpened());

  cv::VideoWriter writer;
  const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();
  if (!save_video) {
    cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
  }

  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                   graph.AddOutputStreamPoller(kOutputStream));

//  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller detectionPoller,
//          graph.AddOutputStreamPoller(KOutputDetection));

//  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller detectionImagePoller,
//          graph.AddOutputStreamPoller(kOutputDetectionVideo));

  MP_RETURN_IF_ERROR(graph.StartRun({}));

  LOG(INFO) << "Start grabbing and processing frames.";
  bool grab_frames = true;
  size_t cur_time_us = 1;
  size_t time_gap_us = 50000; // 50 ms -> 20 fps
  int obj_id = -1;
  int lost_frames = 0;
  int MAX_LOST_FRAMES = 25;
  bool track_lost = false;
  while (grab_frames) {
    // Capture opencv camera or video frame.
    cv::Mat camera_frame_raw;
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) {
      if (!load_video) {
        LOG(INFO) << "Ignore empty frames from camera.";
        continue;
      }
      LOG(INFO) << "Empty frame, end of video reached.";
      break;
    }
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (!load_video) {
      cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }

    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Send image packet into the graph.
//    size_t frame_timestamp_us =
//        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;

//    LOG(INFO) << "frame_timestamp_us: " << frame_timestamp_us;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
        kInputStream, mediapipe::Adopt(input_frame.release())
                          .At(mediapipe::Timestamp(cur_time_us))));

    cur_time_us = cur_time_us + time_gap_us;
//    cv::waitKey(40); // wait for graph processing.

    // Get the graph result packet, or stop if that fails.
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    auto& output_frame = packet.Get<mediapipe::ImageFrame>();

    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);

    // Convert back to opencv for display or saving.
//    cv::Mat output_detection_frame_mat = mediapipe::formats::MatView(&output_frame_detection);
//    cv::cvtColor(output_detection_frame_mat, output_detection_frame_mat, cv::COLOR_RGB2BGR);
    if (save_video) {
      if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(absl::GetFlag(FLAGS_output_video_path) + ".mp4",
                    mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
      }
      writer.write(output_frame_mat);
//      if(track_lost) {
//        writer.write(camera_frame_raw);
//      } else {
//        writer.write(output_frame_mat);
//      }
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;
    }
  }

  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();

  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  auto result = graph.WaitUntilDone();

  LOG(INFO) << "Prepare csv writer.";
  LOG(INFO) << "detections_with_id_packets size: "<<detections_with_id_packets.size();
  std::ofstream fileWriter;
  fileWriter.open(absl::GetFlag(FLAGS_output_video_path) + ".csv");
  fileWriter << "frame,detection_id,track_box,detection_box\n";
  int cur_detection_index = 0;
  for(int i=0; i<tracked_detections_packets.size(); i++) {
    auto& detection_packet = tracked_detections_packets[i];
    auto& out_detections = detection_packet.Get<std::vector<mediapipe::Detection>>();
    auto timestamp = detection_packet.Timestamp();
    std::string origin_detection_str = "";

    if(cur_detection_index < detections_with_id_packets.size()) {
      auto& origin_detection_packet = detections_with_id_packets[cur_detection_index];
      auto t1 = origin_detection_packet.Timestamp();
      if(t1 <= timestamp) {
        cur_detection_index += 1;
        auto& origin_detections = origin_detection_packet.Get<std::vector<mediapipe::Detection>>();
        for(int i=0; i<origin_detections.size(); i++) {
          origin_detection_str += getBBoxString(origin_detections[i]);
          if (i != origin_detections.size() - 1) {
            origin_detection_str += "|";
          }
        }
//        if(origin_detections.size() > 0) {
//          origin_detection_str = getBBoxString(origin_detections[0]);
//        }
      }
    }


    if(out_detections.size() > 0) {
      auto& detection = out_detections[0];
      lost_frames = 0;
      obj_id = detection.detection_id();
      auto detection_str = getBBoxString(detection);

      std::string line = std::to_string((timestamp.Value()/time_gap_us)) + ',' + std::to_string(obj_id) + ","
              + detection_str + "," + origin_detection_str + "\n";

      fileWriter << line;
    } else {
      fileWriter << std::to_string((timestamp.Value()/time_gap_us)) + ",,," + origin_detection_str + "\n";
      if(obj_id > -1) {
        LOG(INFO) << "FRAME LOST !!!!!!!";
        lost_frames += 1;
        if(lost_frames >= MAX_LOST_FRAMES) {
          track_lost = true;
        }
      }
    }
  }

  if(fileWriter.is_open()) fileWriter.close();
  return result;
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
