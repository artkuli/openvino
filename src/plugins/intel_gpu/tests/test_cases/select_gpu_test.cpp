// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "test_utils.h"

#include <intel_gpu/primitives/input_layout.hpp>
#include "intel_gpu/primitives/select.hpp"

using namespace cldnn;
using namespace ::tests;

// select_gpu_f32
TEST(select_gpu_f32, select_basic) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb, { 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
         5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values(mask, {
        0.f,   0.f,  0.f,  0.f,
        1.f,   1.f,  1.f,  1.f,
        0.f,   1.f,  0.f,  1.f,
        1.f,   0.f,  1.f,  0.f });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {  0.5f,  2.5f,   0.5f,  2.5f,
                           2.f,   0.f,    6.f,   5.2f,
                          15.f,   0.5f,   8.f,  12.f,
                           4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_negative) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values(mask, {
        -0.f,   -0.f,  -0.f,  -0.f,
        -1.f,   -1.f,  -1.f,  -1.f,
        -0.f,   -1.f,  -0.f,  -1.f,
        -1.f,   -0.f,  -1.f,  -0.f });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_mask_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 1, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        2.f,  0.f,
        6.f,  5.2f,

        3.f,  0.5f,
        7.f,  12.f,

        4.f,  -0.5f,
        8.f,  8.f
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f,
        0.f,

        1.f,
        1.f,

        0.f,
        1.f,

        1.f,
        0.f,
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        1.5f,  3.f,

        2.f,   0.f,
        6.f,   5.2f,

        15.f,  17.f,
        7.f,   12.f,

        4.f,   -0.5f,
        -0.5f, -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_mask_1x1x1x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 1, 1, 1, 1 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        2.f,  0.f,
        6.f,  5.2f,

        3.f,  0.5f,
        7.f,  12.f,

        4.f,  -0.5f,
        8.f,  8.f
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma_byxf_2x2x2x2_bcast_mask_2x2x2x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf ,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 1 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,   0.f,
        5.f,   1.5f,

        2.f,   0.f,
        6.f,   5.2f,

        3.f,   0.5f,
        7.f,   12.f,

        4.f,   -0.5f,
        8.f,   8.f
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.1f,  0.0f,
        0.5f,  0.0f,

        -0.f,  -0.1f,
        -0.f,  -0.5f,
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        1.f,  2.5f,
        5.f,  3.f,

        2.f,  7.f,
        6.f,  4.f,

        15.f, 0.5f,
        8.f,  12.f,

        -2.f, -0.5f,
        -0.5f, 8.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_in2_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 1, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        2.f,  0.f,
        6.f,  5.2f,

        3.f,  0.5f,
        7.f,  12.f,

        4.f,  -0.5f,
        8.f,  8.f
    });

    set_values(input2, {
        0.5f,
        1.5f,

        5.f,
        2.f,

        15.f,
        8.f,

        -2.f,
        -0.5f,
    });

    set_values(mask, {
        0.f,  0.f,
        0.f,  0.f,

        1.f,  1.f,
        1.f,  1.f,

        0.f,  1.f,
        0.f,  1.f,

        1.f,  0.f,
        1.f,  0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  0.5f,
        1.5f,  1.5f,

        2.f,   0.f,
        6.f,   5.2f,

        15.f,  0.5f,
        8.f,   12.f,

        4.f,   -2.f,
        8.f,   -0.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_in1_2x2x2x1_bcast_in2_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 1, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,

        2.f,  0.f,

        3.f,  0.5f,

        4.f,  -0.5f,
    });

    set_values(input2, {
        0.5f,
        1.5f,

        5.f,
        2.f,

        15.f,
        8.f,

        -2.f,
        -0.5f,
    });

    set_values(mask, {
        0.f,  0.f,
        0.f,  0.f,

        1.f,  1.f,
        1.f,  1.f,

        0.f,  1.f,
        0.f,  1.f,

        1.f,  0.f,
        1.f,  0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  0.5f,
        1.5f,  1.5f,

        2.f,   0.f,
        2.f,   0.f,

        15.f,  0.5f,
        8.f,   0.5f,

        4.f,   -2.f,
        4.f,   -0.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_mask_2x1x2x2_in1_1x2x2x2_in2_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 1, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        2.f,  0.f,
        6.f,  5.2f
    });

    set_values(input2, {
        0.5f,
        1.5f,

        5.f,
        2.f,

        15.f,
        8.f,

        -2.f,
        -0.5f,
    });

    set_values(mask, {
        1.f,  0.f,
        1.f,  0.f,

        0.f,  1.f,
        0.f,  1.f,
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        1.f,   0.5f,
        5.f,   1.5f,

        2.f,   5.f,
        6.f,   2.f,

        15.f,  0.f,
        8.f,   1.5f,

        -2.f,  0.f,
        -0.5f, 5.2f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma_byxf_2x2x2x2_bcast_mask_2x1x2x2_in1_2x2x2x1_in2_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf ,{ 2, 2, 1, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        3.f,  0.5f,
        7.f,  12.f,
    });

    set_values(input2, {
        0.5f,  2.5f,

        5.f,   7.f,

        15.f,  17.f,

        -2.f,  6.5f,
    });

    set_values(mask, {
        0.f,
        0.f,

        0.1f,
        0.5f,

        -0.f,
        -0.5f,

        -0.7f,
        -0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        0.5f,  2.5f,

        1.f,   0.f,
        5.f,   1.5f,

        15.f,  17.f,
        7.f,   12.f,

        3.f,   0.5f,
        -2.f,  6.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_in2_1x1x1x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 1, 1 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        2.f,  0.f,
        6.f,  5.2f,

        3.f,  0.5f,
        7.f,  12.f,

        4.f,  -0.5f,
        8.f,  8.f
    });

    set_values(input2, {
        1.f
    });

    set_values(mask, {
        0.f,  0.f,
        0.f,  0.f,

        1.f,  1.f,
        1.f,  1.f,

        0.f,  1.f,
        0.f,  1.f,

        1.f,  0.f,
        1.f,  0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        1.f,   1.f,
        1.f,   1.f,

        2.f,   0.f,
        6.f,   5.2f,

        1.f,   0.5f,
        1.f,   12.f,

        4.f,   1.f,
        8.f,   1.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma_byxf_2x2x2x2_bcast_in2_2x2x2x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf ,{ 2, 2, 2, 1 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,   0.f,
        5.f,   1.5f,

        2.f,   0.f,
        6.f,   5.2f,

        3.f,   0.5f,
        7.f,   12.f,

        4.f,   -0.5f,
        8.f,   8.f
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        15.f,  17.f,
        8.f,   10.f,
    });

    set_values(mask, {
        0.1f,  0.3f,
        0.5f,  0.7f,

        0.f,   0.f,
        0.f,   0.f,

        -0.f,  -0.1f,
        -0.f,  -0.5f,

        -0.7f, -0.f,
        -1.5f, -0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        1.f,  0.f,
        5.f,  1.5f,

        0.5f, 2.5f,
        1.5f, 3.f,

        15.f, 0.5f,
        8.f,  12.f,

        4.f,  17.0f,
        8.f,  10.0f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_in1_2x2x1x2) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 1, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,
        5.f,

        2.f,
        6.f,

        3.f,
        7.f,

        4.f,
        8.f,
    });

    set_values(input2, {
        0.5f, 2.5f,
        1.5f, 1.f,

        5.f,  7.f,
        2.f,  4.f,

        15.f, 17.f,
        8.f,  10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f,  0.f,
        0.f,  0.f,

        1.f,  1.f,
        1.f,  1.f,

        0.f,  1.f,
        0.f,  1.f,

        1.f,  0.f,
        1.f,  0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        1.5f,  1.f,

        2.f,   2.f,
        6.f,   6.f,

        15.f,  3.f,
        8.f,   7.f,

        4.f,   6.5f,
        8.f,   -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_2x2x2x2_bcast_in1_1x1x1x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 1, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f
    });

    set_values(input2, {
        0.5f, 2.5f,
        1.5f, 1.f,

        5.f,  7.f,
        2.f,  4.f,

        15.f, 17.f,
        8.f,  10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f,  0.f,
        0.f,  0.f,

        1.f,  1.f,
        1.f,  1.f,

        0.f,  1.f,
        0.f,  1.f,

        1.f,  0.f,
        1.f,  0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        1.5f,  1.f,

        1.f,   1.f,
        1.f,   1.f,

        15.f,  1.f,
        8.f,   1.f,

        1.f,   6.5f,
        1.f,   -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma_byxf_2x2x2x2_bcast_in1_2x2x2x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf ,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        3.f,  0.5f,
        7.f,  12.f,
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f,   0.f,
        0.f,   0.f,

        0.1f,  0.3f,
        0.5f,  0.7f,

        -0.f,  -0.1f,
        -0.f,  -0.5f,

        -0.7f, -0.f,
        -1.5f, -0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f, 2.5f,
        1.5f, 3.f,

        1.f,  0.f,
        5.f,  1.5f,

        15.f, 0.5f,
        8.f,  12.f,

        3.f,  6.5f,
        7.f,  -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma_byxf_2x2x2x2_bcast_mask_2x1x2x2_in1_2x2x2x1) {
    auto& engine = get_test_engine();

    auto input1 = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 2, 2, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf ,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf, { 2, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input1", input1->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input1", "input2"));

    set_values(input1, {
        1.f,  0.f,
        5.f,  1.5f,

        3.f,  0.5f,
        7.f,  12.f,
    });

    set_values(input2, {
        0.5f,  2.5f,
        1.5f,  3.f,

        5.f,   7.f,
        2.f,   4.f,

        15.f,  17.f,
        8.f,   10.f,

        -2.f,  6.5f,
        -0.5f, -2.5f
    });

    set_values(mask, {
        0.f,
        0.f,

        0.1f,
        0.5f,

        -0.f,
        -0.5f,

        -0.7f,
        -0.f
    });

    network network(engine, topology);

    network.set_input_data("input1", input1);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = {
        0.5f,  2.5f,
        1.5f,  3.f,

        1.f,   0.f,
        5.f,   1.5f,

        15.f,  17.f,
        7.f,   12.f,

        3.f,   0.5f,
        -0.5f, -2.5f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_comma) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values(mask, {
        0.f,   0.f,  0.f,  0.f,
        0.1f,   0.3f,  0.5f,  0.7f,
        -0.f,   -0.1f,  -0.f,  -0.5f,
        -0.7f,   -0.f,  -1.5f,  -0.f });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_error_input_sizes) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 3, 4, 5, 6 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    EXPECT_ANY_THROW(network(engine, topology));
}

TEST(select_gpu_f32, select_basic_error_mask_sizes) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 3, 4, 5, 6 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    EXPECT_ANY_THROW(network(engine, topology));
}

TEST(select_gpu_f32, select_basic_error_input_types) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::i8, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));
    EXPECT_ANY_THROW(network(engine, topology));
}

TEST(select_gpu_f32, select_basic_error_input_formats) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    EXPECT_ANY_THROW(network(engine, topology));
}

TEST(select_gpu_f32, select_basic_byxf) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::byxf,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values(mask, {
        0.f,   0.f,  0.f,  0.f,
        1.f,   1.f,  1.f,  1.f,
        0.f,   1.f,  0.f,  1.f,
        1.f,   0.f,  1.f,  0.f });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_mask_f16) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f16, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values<uint16_t>(mask, {
        0,   0,  0,  0,
        1,   1,  1,  1,
        0,   1,  0,  1,
        1,   0,  1,  0 });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_mask_i8) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::i8, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values<char>(mask, {
        0,   0,  0,  0,
        1,   1,  1,  1,
        0,   1,  0,  1,
        1,   0,  1,  0 });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_mask_u8) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 2, 2, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::u8, format::yxfb,{ 2, 2, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f, 5.f, 1.5f,
        2.f,   0.f, 6.f, 5.2f,
        3.f,  0.5f, 7.f, 12.f,
        4.f, -0.5f, 8.f,  8.f
    });

    set_values(input2, {
        0.5f,   2.5f,  0.5f,  2.5f,
        5.f,   7.f,    2.f,   4.f,
        15.f,  17.f,    8.f,  10.f,
        -2.f,  6.5f,  -0.5f, -2.5f });

    set_values<unsigned char>(mask, {
        0,   0,  0,  0,
        128,   210,  150,  177,
        0,   211,  0,  255,
        199,   0,  160,  0 });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[16] = { 0.5f,  2.5f,   0.5f,  2.5f,
        2.f,   0.f,    6.f,   5.2f,
        15.f,   0.5f,   8.f,  12.f,
        4.f,   6.5f,   8.f,  -2.5f };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 16; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,    0.f,    2.f,    0.f
    });

    set_values(input2, {
        0.5f,    2.5f,    5.f,    7.f
    });

    set_values(mask, {
        0.f,    0.f,    1.f,    1.f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[4] = {
        0.5f,    2.5f,    2.f,    0.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_bfyx_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::bfyx,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::bfyx,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f,
        2.f,   0.f
    });

    set_values(input2, {
        0.5f,   2.5f,
        5.f,   7.f
    });

    set_values(mask, {
        0.f,   0.f,
        1.f,   1.f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[4] = {
        0.5f,  2.5f,
        2.f,   0.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f32, select_basic_byxf_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::byxf,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::byxf,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::byxf,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,   0.f,
        2.f,   0.f
    });

    set_values(input2, {
        0.5f,   2.5f,
        5.f,   7.f
    });

    set_values(mask, {
        0.f,   0.f,
        1.f,   1.f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[4] = {
        0.5f,  2.5f,
        2.f,   0.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

// select_gpu_f16
TEST(select_gpu_f16, select_basic_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<uint16_t>(input, {
        1,   0,
        2,   0
    });

    set_values<uint16_t>(input2, {
        0,   2,
        5,   7
    });

    set_values<uint16_t>(mask, {
        0,   0,
        1,   1
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    uint16_t answers[4] = {
        0,  2,
        2,   0
    };

    cldnn::mem_lock<uint16_t> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f16, select_basic_mask_f32_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<uint16_t>(input, {
        1,   0,
        2,   0
    });

    set_values<uint16_t>(input2, {
        0,   2,
        5,   7
    });

    set_values<float>(mask, {
        0.f,   0.f,
        1.5f,   0.4f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    uint16_t answers[4] = {
        0,  2,
        2,   0
    };

    cldnn::mem_lock<uint16_t> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f16, select_basic_mask_i8_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<uint16_t>(input, {
        1,   0,
        2,   0
    });

    set_values<uint16_t>(input2, {
        0,   2,
        5,   7
    });

    set_values<char>(mask, {
        0,   0,
        1,   1
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    uint16_t answers[4] = {
        0,  2,
        2,   0
    };

    cldnn::mem_lock<uint16_t> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

TEST(select_gpu_f16, select_basic_mask_u8_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<uint16_t>(input, {
        1,   0,
        2,   0
    });

    set_values<uint16_t>(input2, {
        0,   2,
        5,   7
    });

    set_values<unsigned char>(mask, {
        0,   0,
        128,   255
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    uint16_t answers[4] = {
        0,  2,
        2,   0
    };

    cldnn::mem_lock<uint16_t> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_TRUE(are_equal(answers[i], output_ptr[i]));
    }
}

// select_gpu_i8
TEST(select_gpu_i8, select_basic_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<char>(input, {
        1,   0,
        2,   0
    });

    set_values<char>(input2, {
        0,   2,
        5,   7
    });

    set_values<char>(mask, {
        0,   0,
        3,   5
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    int answers[4] = {
        0,  2,
        2,  0
    };

    cldnn::mem_lock<char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_i8, select_basic_mask_f32_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<char>(input, {
        1,   0,
        2,   0
    });

    set_values<char>(input2, {
        0,   2,
        5,   7
    });

    set_values<float>(mask, {
        0.f,   0.f,
        1.5f,  0.4f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    int answers[4] = {
        0,  2,
        2,  0
    };

    cldnn::mem_lock<char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_i8, select_basic_mask_f16_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<char>(input, {
        1,   0,
        2,   0
    });

    set_values<char>(input2, {
        0,   2,
        5,   7
    });

    set_values<uint16_t>(mask, {
        0,   0,
        3,   5
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    int answers[4] = {
        0,  2,
        2,  0
    };

    cldnn::mem_lock<char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_i8, select_basic_mask_u8_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<char>(input, {
        1,   0,
        2,   0
    });

    set_values<char>(input2, {
        0,   2,
        5,   7
    });

    set_values<unsigned char>(mask, {
        0,   0,
        128,   255
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    int answers[4] = {
        0,  2,
        2,  0
    };

    cldnn::mem_lock<char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

// select_gpu_u8
TEST(select_gpu_u8, select_basic_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<unsigned char>(input, {
        128,   0,
        255,   0
    });

    set_values<unsigned char>(input2, {
        0,   255,
        205,   128
    });

    set_values<unsigned char>(mask, {
        0,   0,
        128,   255
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    unsigned char answers[4] = {
        0,  255,
        255,  0
    };

    cldnn::mem_lock<unsigned char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_u8, select_basic_mask_f32_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f32, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<unsigned char>(input, {
        128,   0,
        255,   0
    });

    set_values<unsigned char>(input2, {
        0,   255,
        205,   128
    });

    set_values<float>(mask, {
        0.f,   0.f,
        1.5f,  0.4f
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    int answers[4] = {
        0,  255,
        255,  0
    };

    cldnn::mem_lock<unsigned char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_u8, select_basic_mask_f16_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::f16, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<unsigned char>(input, {
        128,   0,
        255,   0
    });

    set_values<unsigned char>(input2, {
        0,   255,
        205,   128
    });

    set_values<uint16_t>(mask, {
        0,   0,
        1,   1
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    unsigned char answers[4] = {
        0,  255,
        255,  0
    };

    cldnn::mem_lock<unsigned char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_u8, select_basic_mask_i8_1x1x2x2) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto input2 = engine.allocate_memory({ data_types::u8, format::yxfb,{ 1, 1, 2, 2 } });
    auto mask = engine.allocate_memory({ data_types::i8, format::yxfb,{ 1, 1, 2, 2 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values<unsigned char>(input, {
        128,   0,
        255,   0
    });

    set_values<unsigned char>(input2, {
        0,   255,
        205,   128
    });

    set_values<char>(mask, {
        0,   0,
        1,   1
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    unsigned char answers[4] = {
        0,  255,
        255,  0
    };

    cldnn::mem_lock<unsigned char> output_ptr(output, get_test_stream());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}

TEST(select_gpu_fp32, select_numpy_broadcast_mask_u8_1x1x3) {
    auto& engine = get_test_engine();

    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 3, 1, 1 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 3, 1, 1, 1 } });
    auto mask = engine.allocate_memory({ data_types::u8, format::bfyx, { 1, 1, 3, 1 } });

    topology topology;
    topology.add(input_layout("input", input->get_layout()));
    topology.add(input_layout("input2", input2->get_layout()));
    topology.add(input_layout("mask", mask->get_layout()));
    topology.add(cldnn::select("select", "mask", "input", "input2"));

    set_values(input, {
        1.f,    0.f,    2.f
    });

    set_values(input2, {
        0.5f,    2.5f,    5.f
    });

    set_values<unsigned char>(mask, {
        1,   0,   1
    });

    network network(engine, topology);

    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("mask", mask);
    auto outputs = network.execute();

    auto output = outputs.at("select").get_memory();

    float answers[27] = {
        1.f, 0.5f, 1.f,
        0.f, 0.5f, 0.f,
        2.f, 0.5f, 2.f,
        1.f, 2.5f, 1.f,
        0.f, 2.5f, 0.f,
        2.f, 2.5f, 2.f,
        1.f, 5.f, 1.f,
        0.f, 5.f, 0.f,
        2.f, 5.f, 2.f
    };

    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    for (int i = 0; i < 27; i++)
    {
        EXPECT_EQ(answers[i], output_ptr[i]);
    }
}