//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <mindalpha/model_metric_buffer.h>

namespace mindalpha
{

void ModelMetricBuffer::UpdateBuffer(pybind11::array_t<int64_t> positive_buffer,
                                     pybind11::array_t<int64_t> negative_buffer,
                                     pybind11::array_t<float> predictions,
                                     pybind11::array_t<int64_t> labels)
{
    const size_t buffer_size = positive_buffer.size();
    const size_t instance_count = labels.size();
    int64_t* const pos_buf = positive_buffer.mutable_data();
    int64_t* const neg_buf = negative_buffer.mutable_data();
    const float* const preds = predictions.data();
    const int64_t* const labs = labels.data();
    for (size_t i = 0; i < instance_count; i++)
    {
        const float pred = preds[i];
        const int64_t lab = labs[i];
        const int64_t bucket = static_cast<int64_t>(pred * (buffer_size - 1));
        if (lab > 0)
            pos_buf[bucket] += lab;
        else if (lab < 0)
            neg_buf[bucket] += -lab;
        else
            neg_buf[bucket] += 1;
    }
}

double ModelMetricBuffer::ComputeAUC(pybind11::array_t<int64_t> positive_buffer,
                                     pybind11::array_t<int64_t> negative_buffer)
{
    const size_t buffer_size = positive_buffer.size();
    const int64_t* const pos_buf = positive_buffer.mutable_data();
    const int64_t* const neg_buf = negative_buffer.mutable_data();
    double auc = 0.0;
    int64_t prev_pos_sum = 0;
    int64_t pos_sum = 0;
    int64_t neg_sum = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        const int64_t pos = pos_buf[i];
        const int64_t neg = neg_buf[i];
        prev_pos_sum = pos_sum;
        pos_sum += pos;
        neg_sum += neg;
        auc += 0.5 * (prev_pos_sum + pos_sum) * neg;
    }
    if (pos_sum == 0)
        auc = (neg_sum > 0) ? 0.0 : 1.0;
    else if (neg_sum == 0)
        auc = 1.0;
    else
        auc = 1.0 - auc / pos_sum / neg_sum;
    return auc;
}

}
