// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <../api/engine.hpp>
#include <../api/input_layout.hpp>
#include <../api/memory.hpp>
#include <../api/data.hpp>
#include <../api/topology.hpp>
#include <../api/network.hpp>
#include <iostream>

#include "helper_functions.h"

/*! @page c4 Hidden layers.
* @section intro Introduction
* In this chapter we show, how to get access to hidden layers using build options 
*
*
* @include chapter_4.cpp
*
*
*/

using namespace cldnn;


void chapter_4(engine& engine, topology& topology)
{

    std::cout << std::endl << "-- Chapter 4 --" << std::endl;

    // To get access to intermediate results of our network. To get special features we need to set custom building options:
    build_options build_opt;
    // Prepare vector of primitives that we want to have as an output:
    std::vector<cldnn::primitive_id> outputs_list(0);
    // Put every primitive from topology into this container:
    for (auto prim_id : topology.get_primitive_ids())
        outputs_list.push_back(prim_id);
    // Note: output from get_primitive_ids() can be used directly as a parameter in building option.
    // Set option.
    build_opt.set_option(build_option::outputs(outputs_list));
    // Add build options to network build.
    network network(engine, topology, build_opt);
    // We are almost ready to go. Need to create and set input for network:
    memory input_prim = memory::allocate(engine, { data_types::f32, format::bfyx,{ 1, 1, 3, 1 } });
    set_values(input_prim, { -3.0f, -2.0f, 2.5f });
    // Set input.
    network.set_input_data("input", input_prim);
    // Ready to go:
    auto outputs = network.execute();

    for (auto& it : outputs)
    {
        // Print id and output values.
        std::cout << it.first << std::endl;
        auto mem_pointer = it.second.get_memory().pointer<float>();
        for (auto i : mem_pointer)
        {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
}
