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

#include "mediapipe/calculators/tflite/tflite_tensors_to_toy_detection_calculator.pb.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/location.h"
#include "mediapipe/framework/port/ret_check.h"
#include "tensorflow/lite/interpreter.h"

namespace mediapipe {

    namespace {
        inline float getValFromBuffer(const float* array, int channel, int row, int col, int row_size, int col_size, int channel_size) {
            // return array[channel * row_size * col_size + row * col_size + col];
            return array[row * col_size * channel_size + col * channel_size + channel];
        }
    }  // namespace

    class TfLiteTensorsToToyDetectionCalculator : public CalculatorBase {
    public:
        static absl::Status GetContract(CalculatorContract* cc);

        absl::Status Open(CalculatorContext* cc) override;
        absl::Status Process(CalculatorContext* cc) override;

    private:
        absl::Status LoadOptions(CalculatorContext* cc);
        int num_landmarks_ = 0;
        int num_boxes_ = 0;
        int down_ratio_ = 0;
        int input_width_ = 0;
        int input_height_ = 0;
        float min_score_thresh_ = 0.0f;
        bool flip_vertically_ = false;
        bool flip_horizontally_ = false;
        ::mediapipe::TfLiteTensorsToToyDetectionCalculatorOptions options_;
        struct IndexObj {
            int idx;
            int jdx;
            float score;
            int category_id;
        };

        struct CompareIndexObj {
            bool operator() (IndexObj const& left, IndexObj const& right) {
                return left.score < right.score;
            }
        };


        absl::Status ConvertToDetections(const NormalizedLandmarkList normalized_landmark_list,
                                         const float score,
                                         std::vector<Detection>* output_detections);
        absl::Status GetDetectionFromModel(const float* hw2d_buffer, 
                                           int idx, int jdx, int row_size, int col_size, int channel_size, float down_ratio_, float score, int category_id,
                                           std::vector<Detection>* output_detections); 

        absl::Status getLandmarksFromPeaks(const float* center_offset_buffer, const float* hw_buffer, int idx, int jdx, int row_size, int col_size, int channel_size,
                                           LandmarkList* landmark_list_output, NormalizedLandmarkList* normalized_landmark_list_output);

        absl::Status getTopKPeaksFromChannel(const float* hmax_buffer, const float* heatmap_buffer, int channelIdx, int row_size, int col_size, int channel_size,
                                             std::priority_queue<IndexObj, std::vector<IndexObj>, CompareIndexObj>* priority_queue_output);
    };
    REGISTER_CALCULATOR(TfLiteTensorsToToyDetectionCalculator);

    absl::Status TfLiteTensorsToToyDetectionCalculator::GetContract(
            CalculatorContract* cc) {
        RET_CHECK(!cc->Inputs().GetTags().empty());
        RET_CHECK(!cc->Outputs().GetTags().empty());

        if (cc->Inputs().HasTag("TENSORS")) {
            cc->Inputs().Tag("TENSORS").Set<std::vector<TfLiteTensor>>();
        }

        if (cc->Inputs().HasTag("FLIP_HORIZONTALLY")) {
            cc->Inputs().Tag("FLIP_HORIZONTALLY").Set<bool>();
        }

        if (cc->Inputs().HasTag("FLIP_VERTICALLY")) {
            cc->Inputs().Tag("FLIP_VERTICALLY").Set<bool>();
        }

        if (cc->InputSidePackets().HasTag("FLIP_HORIZONTALLY")) {
            cc->InputSidePackets().Tag("FLIP_HORIZONTALLY").Set<bool>();
        }

        if (cc->InputSidePackets().HasTag("FLIP_VERTICALLY")) {
            cc->InputSidePackets().Tag("FLIP_VERTICALLY").Set<bool>();
        }

//        if (cc->Outputs().HasTag("LANDMARKS")) {
//            cc->Outputs().Tag("LANDMARKS").Set<LandmarkList>();
//        }
//
//        if (cc->Outputs().HasTag("NORM_LANDMARKS")) {
//            cc->Outputs().Tag("NORM_LANDMARKS").Set<NormalizedLandmarkList>();
//        }

        if (cc->Outputs().HasTag("DETECTIONS")) {
            cc->Outputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        }

        return absl::OkStatus();
    }

    absl::Status TfLiteTensorsToToyDetectionCalculator::Open(CalculatorContext* cc) {
        cc->SetOffset(TimestampDiff(0));

        MP_RETURN_IF_ERROR(LoadOptions(cc));

        flip_horizontally_ =
                cc->InputSidePackets().HasTag("FLIP_HORIZONTALLY")
                ? cc->InputSidePackets().Tag("FLIP_HORIZONTALLY").Get<bool>()
                : options_.flip_horizontally();

        flip_vertically_ =
                cc->InputSidePackets().HasTag("FLIP_VERTICALLY")
                ? cc->InputSidePackets().Tag("FLIP_VERTICALLY").Get<bool>()
                : options_.flip_vertically();

        return absl::OkStatus();
    }

    absl::Status TfLiteTensorsToToyDetectionCalculator::Process(
            CalculatorContext* cc) {
        // Override values if specified so.
        if (cc->Inputs().HasTag("FLIP_HORIZONTALLY") &&
            !cc->Inputs().Tag("FLIP_HORIZONTALLY").IsEmpty()) {
            flip_horizontally_ = cc->Inputs().Tag("FLIP_HORIZONTALLY").Get<bool>();
        }
        if (cc->Inputs().HasTag("FLIP_VERTICALLY") &&
            !cc->Inputs().Tag("FLIP_VERTICALLY").IsEmpty()) {
            flip_vertically_ = cc->Inputs().Tag("FLIP_VERTICALLY").Get<bool>();
        }

        if (cc->Inputs().Tag("TENSORS").IsEmpty()) {
            return absl::OkStatus();
        }

        const auto& input_tensors =
                cc->Inputs().Tag("TENSORS").Get<std::vector<TfLiteTensor>>();

        RET_CHECK(input_tensors.size()==3) <<" input tensor size must be 3";
        int row_size = input_height_ / down_ratio_;
        int col_size = input_width_ / down_ratio_;

        const TfLiteTensor* wh2d_tensor = &input_tensors[0];
        CHECK_EQ(wh2d_tensor->dims->size, 4);
        CHECK_EQ(wh2d_tensor->dims->data[0], 1);
        int wh2d_channel_size = wh2d_tensor->dims->data[3];
        // CHECK_EQ(wh2d_channel_size, 4);  // 2*2 = 4
        int wh2d_row_size = wh2d_tensor->dims->data[1];
        CHECK_EQ(wh2d_row_size, row_size);
        int wh2d_col_size = wh2d_tensor->dims->data[2];
        CHECK_EQ(wh2d_col_size, col_size);
        const float* wh2d_buffer = wh2d_tensor->data.f;

        const TfLiteTensor* heatmap2d_tensor = &input_tensors[1];
        CHECK_EQ(heatmap2d_tensor->dims->size, 4);
        CHECK_EQ(heatmap2d_tensor->dims->data[0], 1);
        CHECK_EQ(heatmap2d_tensor->dims->data[3], 126);
        int heatmap_row_size = heatmap2d_tensor->dims->data[1];
        CHECK_EQ(heatmap_row_size, row_size);
        int heatmap_col_size = heatmap2d_tensor->dims->data[2];
        CHECK_EQ(heatmap_col_size, col_size);
        const float* heatmap_buffer = heatmap2d_tensor->data.f;

        const TfLiteTensor* h2dmax_tensor = &input_tensors[2];
        CHECK_EQ(h2dmax_tensor->dims->size, 4);
        CHECK_EQ(h2dmax_tensor->dims->data[0], 1);
        CHECK_EQ(h2dmax_tensor->dims->data[3], 126);
        int hmax_row_size = h2dmax_tensor->dims->data[1];
        CHECK_EQ(hmax_row_size, row_size);
        int hmax_col_size = h2dmax_tensor->dims->data[2];
        CHECK_EQ(hmax_col_size, col_size);
        const float* hmax_buffer = h2dmax_tensor->data.f;

        std::priority_queue<IndexObj, std::vector<IndexObj>, CompareIndexObj> priority_queue_;
        int channel_size = 1; // Heatmap only uses 1 channel for now.
        getTopKPeaksFromChannel(hmax_buffer, heatmap_buffer, 0, row_size, col_size, channel_size, &priority_queue_);


        auto output_detections = absl::make_unique<std::vector<Detection>>();
        int num_boxes = num_boxes_ < priority_queue_.size() ? num_boxes_: priority_queue_.size();
        // LOG(INFO) << "number of peaks: " << priority_queue_.size();
        for (int i=0; i<num_boxes; i++) {
            if(priority_queue_.size() > 0) {
                IndexObj obj = priority_queue_.top();
                // LOG(INFO) << "score: " << obj.score <<" idx: " << obj.idx <<" jdx: "<<obj.jdx;

                // find 8 landmark points
                // LandmarkList output_landmarks;
                // NormalizedLandmarkList output_norm_landmarks;
                // MP_RETURN_IF_ERROR(getLandmarksFromPeaks(center_offset_buffer, hw_buffer, obj.idx, obj.jdx, row_size, col_size,
                //                       &output_landmarks, &output_norm_landmarks));

                // add to detection output
                int wh_channel_size = 4; // for 2 top-left and bottom-right points
                MP_RETURN_IF_ERROR(GetDetectionFromModel(
                    wh2d_buffer,
                    obj.idx, obj.jdx, row_size, col_size, wh_channel_size,
                    down_ratio_,
                    obj.score,
                    obj.category_id,
                    output_detections.get()
                ));
                priority_queue_.pop();
            }
        }


        if (cc->Outputs().HasTag("DETECTIONS")) {
//            LOG(INFO)<<"output detection: "<<output_detections->size();
            cc->Outputs()
                .Tag("DETECTIONS")
                .Add(output_detections.release(), cc->InputTimestamp());
        }

//        if (cc->Outputs().HasTag("NORM_LANDMARKS")) {
//            cc->Outputs()
//                    .Tag("NORM_LANDMARKS")
//                    .AddPacket(MakePacket<NormalizedLandmarkList>(output_norm_landmarks)
//                                       .At(cc->InputTimestamp()));
//        }
//        // Output absolute landmarks.
//        if (cc->Outputs().HasTag("LANDMARKS")) {
//            cc->Outputs()
//                    .Tag("LANDMARKS")
//                    .AddPacket(MakePacket<LandmarkList>(output_landmarks)
//                                       .At(cc->InputTimestamp()));
//        }

        return absl::OkStatus();
    }

    absl::Status TfLiteTensorsToToyDetectionCalculator::getLandmarksFromPeaks(const float* center_offset_buffer, const float* hw_buffer, int idx, int jdx, int row_size, int col_size, int channel_size,
                          LandmarkList* landmark_list_output, NormalizedLandmarkList* normalized_landmark_list_output) {
        int center_offset_channel = 2;
        const float center_offset_x = getValFromBuffer(center_offset_buffer, 0, idx, jdx, row_size, col_size, center_offset_channel);
        float xs = jdx + center_offset_x;
        const float center_offset_y = getValFromBuffer(center_offset_buffer, 1, idx, jdx, row_size, col_size, center_offset_channel);
        float ys = idx + center_offset_y;
//            LOG(INFO) << "xs: "<< xs << " ys: "<< ys;
        channel_size = 16;
        for(int cidx = 0; cidx < num_landmarks_; ++cidx) {
            float delta_x = getValFromBuffer(hw_buffer, 2*cidx, idx, jdx, row_size, col_size, channel_size);
            float delta_y = getValFromBuffer(hw_buffer, 2*cidx+1, idx, jdx, row_size, col_size, channel_size);
            float x = (xs + delta_x) * down_ratio_;
            float y = (ys + delta_y) * down_ratio_;
            Landmark* landmark = landmark_list_output->add_landmark();
            if (flip_horizontally_) {
                landmark->set_x(input_width_ - x);
            } else {
                landmark->set_x(x);
            }
            if (flip_vertically_) {
                landmark->set_y(input_height_ - y);
            } else {
                landmark->set_y(y);
            }
            NormalizedLandmark* norm_landmark = normalized_landmark_list_output->add_landmark();
            norm_landmark->set_x(landmark->x() / input_width_);
            norm_landmark->set_y(landmark->y() / input_height_);
//                LOG(INFO) <<"index: " << cidx << " x: " << norm_landmark->x()  << " y: " << norm_landmark->y();
        }

        return absl::OkStatus();
    }

    absl::Status TfLiteTensorsToToyDetectionCalculator::getTopKPeaksFromChannel(const float* hmax_buffer, const float* heatmap_buffer, int channelIdx, int row_size, int col_size,
                                                                                int channel_size,
                                                                                std::priority_queue<IndexObj, std::vector<IndexObj>, CompareIndexObj>* priority_queue_output) {
      channel_size = 126;
      for (int channelIdx_=0; channelIdx_ < 126; channelIdx_ ++) {
        for(int idx=0; idx<row_size; idx++) {
            for(int jdx=0; jdx<col_size; jdx++) {
                const float hmax_score = getValFromBuffer(hmax_buffer, channelIdx_, idx, jdx, row_size, col_size, channel_size);
                const float heatmap_score = getValFromBuffer(heatmap_buffer, channelIdx_, idx, jdx, row_size, col_size, channel_size);
                if(hmax_score == heatmap_score && heatmap_score > min_score_thresh_) {
                    priority_queue_output->push({idx, jdx, heatmap_score, channelIdx_});
                }
            }
        }
      }
      return absl::OkStatus();
    }


    absl::Status TfLiteTensorsToToyDetectionCalculator::ConvertToDetections(const NormalizedLandmarkList normalized_landmark_list,
                                     const float score,
                                     std::vector<Detection>* output_detections) {
        float min_x=1.0f, min_y=1.0f;
        float max_x=0.0f, max_y=0.0f;
        for(int i=0; i<normalized_landmark_list.landmark_size(); i++) {
            const NormalizedLandmark& norm_landmark = normalized_landmark_list.landmark(i);
            float x_val = norm_landmark.x();
            float y_val = norm_landmark.y();
            if(x_val < min_x) {
                min_x = x_val;
            }
            if(x_val > max_x) {
                max_x = x_val;
            }
            if(y_val < min_y) {
                min_y = y_val;
            }
            if(y_val > max_y) {
                max_y = y_val;
            }
        }
//        LOG(INFO) <<"min_x: "<<min_x << " max_x: "<<max_x<<" min_y: "<<min_y<<" max_y:"<<max_y;

        Detection detection;
        detection.add_score(score);
        detection.add_label("toys"); // hard code to toy

        LocationData* location_data = detection.mutable_location_data();
        location_data->set_format(LocationData::RELATIVE_BOUNDING_BOX);

        LocationData::RelativeBoundingBox* relative_bbox =
                location_data->mutable_relative_bounding_box();

        relative_bbox->set_xmin(min_x);
        relative_bbox->set_ymin(min_y);
        relative_bbox->set_width(max_x - min_x);
        relative_bbox->set_height(max_y - min_y);

        output_detections->emplace_back(detection);

        return absl::OkStatus();
    }

      absl::Status TfLiteTensorsToToyDetectionCalculator::GetDetectionFromModel(const float* hw2d_buffer, int idx, int jdx, int row_size, int col_size, int channel_size, float down_ratio_, float score, int category_id, std::vector<Detection>* output_detections) {
        float xs = jdx * down_ratio_;
        float ys = idx * down_ratio_;
        Detection detection;
        detection.add_label_id(category_id);
        detection.add_score(score);
        // detection.add_label("toys");
        LocationData* location_data = detection.mutable_location_data();
        location_data->set_format(LocationData::RELATIVE_BOUNDING_BOX);
        LocationData::RelativeBoundingBox* relative_bbox =
            location_data->mutable_relative_bounding_box();
        float delta_x = getValFromBuffer(hw2d_buffer, 0, idx, jdx, row_size, col_size, channel_size);
        float delta_y = getValFromBuffer(hw2d_buffer, 1, idx, jdx, row_size, col_size, channel_size);
        float min_x = (xs - delta_x); // * down_ratio_;
        if (flip_horizontally_) min_x = input_width_ - min_x;
        float min_y = (ys - delta_y); // * down_ratio_;
        if (flip_vertically_) min_y = input_height_ - min_y;

        delta_x = getValFromBuffer(hw2d_buffer, 2, idx, jdx, row_size, col_size, channel_size);
        delta_y = getValFromBuffer(hw2d_buffer, 3, idx, jdx, row_size, col_size, channel_size);

        float max_x = (xs + delta_x); // * down_ratio_;
        if (flip_horizontally_) max_x = input_width_ - max_x;
        float max_y = (ys + delta_y); // * down_ratio_;
        if (flip_vertically_) max_y = input_height_ - max_y;

        min_x /= input_width_;
        min_y /= input_height_;
        max_x /= input_width_;
        max_y /= input_height_;

        relative_bbox->set_xmin(min_x);
        relative_bbox->set_ymin(min_y);
        relative_bbox->set_width(max_x - min_x);
        relative_bbox->set_height(max_y - min_y);

        output_detections->emplace_back(detection);

        return absl::OkStatus();
      }

    absl::Status TfLiteTensorsToToyDetectionCalculator::LoadOptions(
            CalculatorContext* cc) {
        // Get calculator options specified in the graph.
        options_ =
                cc->Options<::mediapipe::TfLiteTensorsToToyDetectionCalculatorOptions>();
        num_landmarks_ = options_.num_landmarks();
        num_boxes_ = options_.num_boxes();
        min_score_thresh_ = options_.min_score_thresh();
        down_ratio_ = options_.down_ratio();

        RET_CHECK(options_.has_input_image_height() &&
                  options_.has_input_image_width())
                << "Must provide input width/height";
        input_height_ = options_.input_image_height();
        input_width_ = options_.input_image_width();

        return absl::OkStatus();
    }
}  // namespace mediapipe
