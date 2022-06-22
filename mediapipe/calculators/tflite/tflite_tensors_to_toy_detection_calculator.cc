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

        inline float Sigmoid(float value) { return 1.0f / (1.0f + std::exp(-value)); }

        const float CONF_THRESHOLD  = 0.5;
        const float DOWN_RATIO = 8.0;

        inline float getValFromBuffer(const float* array, int channel, int row, int col, int row_size, int col_size) {
            return array[channel * row_size * col_size + row * col_size + col];
        }

        void getMaxArgFromChannel(const float* array, int channelIdx, int row_size, int col_size,
                                  int* best_idx, int* best_jdx, float* best_score) {
//            int best_idx = -1;
//            int best_jdx = -1;
//            float best_score = -1;
            for(int idx=0; idx<row_size; idx++) {
                for(int jdx=0; jdx<col_size; jdx++) {
                    const float score = getValFromBuffer(array, channelIdx, idx, jdx, row_size, col_size);
                    if(score > *best_score && score > CONF_THRESHOLD) {
                        *best_idx = idx;
                        *best_jdx = jdx;
                        *best_score = score;
                    }
                }
            }
        }
    }  // namespace

// A calculator for converting TFLite tensors from regression models into
// landmarks. Note that if the landmarks in the tensor has more than 5
// dimensions, only the first 5 dimensions will be converted to
// [x,y,z, visibility, presence]. The latter two fields may also stay unset if
// such attributes are not supported in the model.
//
// Input:
//  TENSORS - Vector of TfLiteTensor of type kTfLiteFloat32 model output
//
//  FLIP_HORIZONTALLY (optional): Whether to flip landmarks horizontally or
//  not. Overrides corresponding side packet and/or field in the calculator
//  options.
//
//  FLIP_VERTICALLY (optional): Whether to flip landmarks vertically or not.
//  Overrides corresponding side packet and/or field in the calculator options.
//
// Input side packet:
//   FLIP_HORIZONTALLY (optional): Whether to flip landmarks horizontally or
//   not. Overrides the corresponding field in the calculator options.
//
//   FLIP_VERTICALLY (optional): Whether to flip landmarks vertically or not.
//   Overrides the corresponding field in the calculator options.
//
// Output:
//  LANDMARKS(optional) - Result MediaPipe landmarks.
//
// Notes:
//   To output normalized landmarks, user must provide the original input image
//   size to the model using calculator option input_image_width and
//   input_image_height.
// Usage example:
// node {
//   calculator: "TfLiteTensorsToLandmarksCalculator"
//   input_stream: "TENSORS:landmark_tensors"
//   output_stream: "LANDMARKS:landmarks"
//   output_stream: "NORM_LANDMARKS:landmarks"
//   options: {
//     [mediapipe.TfLiteTensorsToLandmarksCalculatorOptions.ext] {
//       num_landmarks: 8
//       input_image_width: 256
//       input_image_height: 256
//     }
//   }
// }
    class TfLiteTensorsToToyDetectionCalculator : public CalculatorBase {
    public:
        static absl::Status GetContract(CalculatorContract* cc);

        absl::Status Open(CalculatorContext* cc) override;
        absl::Status Process(CalculatorContext* cc) override;

    private:
        absl::Status LoadOptions(CalculatorContext* cc);
        int num_landmarks_ = 0;
        bool flip_vertically_ = false;
        bool flip_horizontally_ = false;
        ::mediapipe::TfLiteTensorsToToyDetectionCalculatorOptions options_;

        absl::Status ConvertToDetections(const NormalizedLandmarkList normalized_landmark_list,
                                         const float score,
                                         std::vector<Detection>* output_detections);
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

        if (cc->Outputs().HasTag("LANDMARKS")) {
            cc->Outputs().Tag("LANDMARKS").Set<LandmarkList>();
        }

        if (cc->Outputs().HasTag("NORM_LANDMARKS")) {
            cc->Outputs().Tag("NORM_LANDMARKS").Set<NormalizedLandmarkList>();
        }

        if (cc->Outputs().HasTag("DETECTIONS")) {
            cc->Outputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        }

        return absl::OkStatus();
    }

    absl::Status TfLiteTensorsToToyDetectionCalculator::Open(CalculatorContext* cc) {
        cc->SetOffset(TimestampDiff(0));

        MP_RETURN_IF_ERROR(LoadOptions(cc));

        if (cc->Outputs().HasTag("NORM_LANDMARKS")) {
            RET_CHECK(options_.has_input_image_height() &&
                      options_.has_input_image_width())
                    << "Must provide input width/height for getting normalized landmarks.";
        }
        if (cc->Outputs().HasTag("LANDMARKS") &&
            (options_.flip_vertically() || options_.flip_horizontally() ||
             cc->InputSidePackets().HasTag("FLIP_HORIZONTALLY") ||
             cc->InputSidePackets().HasTag("FLIP_VERTICALLY"))) {
            RET_CHECK(options_.has_input_image_height() &&
                      options_.has_input_image_width())
                    << "Must provide input width/height for using flip_vertically option "
                       "when outputing landmarks in absolute coordinates.";
        }

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

        RET_CHECK(input_tensors.size()==4) <<" input tensor size must be 4";

        const TfLiteTensor* hw_tensor = &input_tensors[0];
        CHECK_EQ(hw_tensor->dims->size, 4);
        CHECK_EQ(hw_tensor->dims->data[0], 1);
        int hw_channel_size = hw_tensor->dims->data[1];
        CHECK_EQ(hw_channel_size, 16);
        int hw_row_size = hw_tensor->dims->data[2];
        CHECK_EQ(hw_row_size, 32);
        int hw_col_size = hw_tensor->dims->data[3];
        CHECK_EQ(hw_col_size, 32);
        const float* hw_buffer = hw_tensor->data.f;

        const TfLiteTensor* center_offset_tensor = &input_tensors[1];
        CHECK_EQ(center_offset_tensor->dims->size, 4);
        CHECK_EQ(center_offset_tensor->dims->data[0], 1);
        CHECK_EQ(center_offset_tensor->dims->data[1], 2);
        int center_offset_row_size = center_offset_tensor->dims->data[2];
        CHECK_EQ(center_offset_row_size, 32);
        int center_offset_col_size = center_offset_tensor->dims->data[3];
        CHECK_EQ(center_offset_col_size, 32);
        const float* center_offset_buffer = center_offset_tensor->data.f;

        const TfLiteTensor* heatmap_tensor = &input_tensors[2];
        CHECK_EQ(heatmap_tensor->dims->size, 4);
        CHECK_EQ(heatmap_tensor->dims->data[0], 1);
        CHECK_EQ(heatmap_tensor->dims->data[1], 1);
        int heatmap_row_size = heatmap_tensor->dims->data[2];
        CHECK_EQ(heatmap_row_size, 32);
        int heatmap_col_size = heatmap_tensor->dims->data[3];
        CHECK_EQ(heatmap_col_size, 32);
        const float* heatmap_buffer = heatmap_tensor->data.f;

        LandmarkList output_landmarks;
        NormalizedLandmarkList output_norm_landmarks;
        auto output_detections = absl::make_unique<std::vector<Detection>>();

        int best_idx = -1;
        int best_jdx = -1;
        float best_score = -1.0f;
        getMaxArgFromChannel(heatmap_buffer, 0, heatmap_row_size, heatmap_col_size, &best_idx, &best_jdx, &best_score);
//        LOG(INFO) << "best_idx: "<< best_idx << " best_jdx: "<< best_jdx << " best_score: " << best_score;

        int input_height = options_.input_image_height();
        int input_width = options_.input_image_width();

        if(best_idx != -1 && best_jdx != -1) {
            const float center_offset_x = getValFromBuffer(center_offset_buffer, 0, best_idx, best_jdx, center_offset_row_size, center_offset_col_size);
            float xs = best_jdx + center_offset_x;
            const float center_offset_y = getValFromBuffer(center_offset_buffer, 1, best_idx, best_jdx, center_offset_row_size, center_offset_col_size);
            float ys = best_idx + center_offset_y;
//            LOG(INFO) << "xs: "<< xs << " ys: "<< ys;

            for(int cidx = 0; cidx < num_landmarks_; ++cidx) {
                float delta_x = getValFromBuffer(hw_buffer, 2*cidx, best_idx, best_jdx, hw_row_size, hw_col_size);
                float delta_y = getValFromBuffer(hw_buffer, 2*cidx+1, best_idx, best_jdx, hw_row_size, hw_col_size);
                float x = (xs + delta_x) * DOWN_RATIO;
                float y = (ys + delta_y) * DOWN_RATIO;
                Landmark* landmark = output_landmarks.add_landmark();
                if (flip_horizontally_) {
                    landmark->set_x(input_width - x);
                } else {
                    landmark->set_x(x);
                }
                if (flip_vertically_) {
                    landmark->set_y(input_height - y);
                } else {
                    landmark->set_y(y);
                }
                // Output normalized landmarks if required.
                NormalizedLandmark* norm_landmark = output_norm_landmarks.add_landmark();
                norm_landmark->set_x(landmark->x() / input_width);
                norm_landmark->set_y(landmark->y() / input_height);
//                LOG(INFO) <<"index: " << cidx << " x: " << norm_landmark->x()  << " y: " << norm_landmark->y();
            }

            MP_RETURN_IF_ERROR(ConvertToDetections(output_norm_landmarks, best_score,
                                                   output_detections.get()));
        }

        if (cc->Outputs().HasTag("DETECTIONS")) {
//            LOG(INFO)<<"output detection: "<<output_detections->size();
            cc->Outputs()
                .Tag("DETECTIONS")
                .Add(output_detections.release(), cc->InputTimestamp());
        }

        if (cc->Outputs().HasTag("NORM_LANDMARKS")) {
            cc->Outputs()
                    .Tag("NORM_LANDMARKS")
                    .AddPacket(MakePacket<NormalizedLandmarkList>(output_norm_landmarks)
                                       .At(cc->InputTimestamp()));
        }
        // Output absolute landmarks.
        if (cc->Outputs().HasTag("LANDMARKS")) {
            cc->Outputs()
                    .Tag("LANDMARKS")
                    .AddPacket(MakePacket<LandmarkList>(output_landmarks)
                                       .At(cc->InputTimestamp()));
        }

//        auto output_tensors = absl::make_unique<std::vector<TfLiteTensor>>();
//        output_tensors->emplace_back(input_tensors[0]);
//        cc->Outputs()
//                .Tag("TENSORS")
//                .Add(output_tensors.release(), cc->InputTimestamp());

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

    absl::Status TfLiteTensorsToToyDetectionCalculator::LoadOptions(
            CalculatorContext* cc) {
        // Get calculator options specified in the graph.
        options_ =
                cc->Options<::mediapipe::TfLiteTensorsToToyDetectionCalculatorOptions>();
        num_landmarks_ = options_.num_landmarks();

        return absl::OkStatus();
    }
}  // namespace mediapipe
