/*
// Copyright (c) 2018 Intel Corporation
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
*/

#include "activation_grad_inst.h"
#include "primitive_type_base.h"
#include "error_handler.h"
#include "json_object.h"
#include <string>

namespace cldnn {
primitive_type_id activation_grad_type_id() {
    static primitive_type_base<activation_grad> instance;
    return &instance;
}

layout activation_grad_inst::calc_output_layout(activation_grad_node const& node) {
    assert(static_cast<bool>(node.get_primitive()->output_data_type) == false &&
           "Output data type forcing is not supported for "
           "activation_grad_node!");
    return node.input().get_non_padded_output_layout();
}

std::string activation_grad_inst::to_string(activation_grad_node const& node) {
    auto node_info = node.desc_to_json();
    auto desc = node.get_primitive();

    std::stringstream primitive_description;

    json_composite activation_grad_info;
    activation_grad_info.add("activation_grad_func", desc->activation_grad_func);
    activation_grad_info.add("additional_params.a", desc->additional_params.a);
    activation_grad_info.add("additional_params.b", desc->additional_params.b);
    activation_grad_info.add("additional_params input", desc->additional_params_input);

    node_info->add("activation_grad info", activation_grad_info);
    node_info->dump(primitive_description);

    return primitive_description.str();
}

activation_grad_inst::typed_primitive_inst(network_impl& network, activation_grad_node const& node)
    : parent(network, node) {
    auto input_grad_arg = node.input().get_output_layout();
    auto input_arg = node.input_arg().get_output_layout();
    auto output_arg = node.get_output_layout();

    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "ReLU input_grad number",
                          input_grad_arg.size.raw.size(),
                          "ReLU input number",
                          input_arg.size.raw.size(),
                          "Relu input_grad/input num dismatch");
    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "ReLU input number",
                          input_arg.size.raw.size(),
                          "ReLU output number",
                          output_arg.size.raw.size(),
                          "Relu input/output num dismatch");

    if (is_parameterized()) {
        /// Slope input x dimension should be equal to input feature size (one slope per channel).
        auto slope_input_size = node.slope_input().get_output_layout().size;
        auto input_feature_size = node.input().get_output_layout().size.feature[0];

        CLDNN_ERROR_LESS_THAN(node.id(),
                              "Slope x size",
                              slope_input_size.spatial[0],
                              "input feature size",
                              input_feature_size,
                              "Dimensions mismatch between input and slope input in Activation layer(slope x size "
                              "should be equal to input feature size)!");

        // All other dimensions should be 1
        CLDNN_ERROR_NOT_EQUAL(node.id(),
                              "Slope input size count",
                              slope_input_size.count(),
                              "Slope input size x",
                              slope_input_size.spatial[0],
                              "Dimensions mismatch of slope input in Activation layer!");
    }
}
}  // namespace cldnn
