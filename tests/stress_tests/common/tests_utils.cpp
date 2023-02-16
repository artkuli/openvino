// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "tests_utils.h"

#include <gtest/gtest.h>
#include <pugixml.hpp>
#include <string>

#define DEBUG_MODE false

const pugi::xml_document & Environment::getTestConfig() {
    return _test_config;
}

void Environment::setTestConfig(const pugi::xml_document &test_config) {
    _test_config.reset(test_config);
}

std::vector<TestCase> generateTestsParams(std::initializer_list<std::string> fields) {
    std::vector<TestCase> tests_cases;
    const pugi::xml_document & test_config = Environment::Instance().getTestConfig();

    std::vector<int> processes, threads, iterations;
    std::vector<std::string> devices, models, models_names, precisions;

    pugi::xml_node values;
    for (auto field = fields.begin(); field != fields.end(); field++) {
        if (*field == "processes") {
            values = test_config.child("attributes").child("processes");
            for (pugi::xml_node val = values.first_child(); val; val = val.next_sibling())
                processes.push_back(val.text().as_int());
        } else if (*field == "threads") {
            values = test_config.child("attributes").child("threads");
            for (pugi::xml_node val = values.first_child(); val; val = val.next_sibling())
                threads.push_back(val.text().as_int());
        } else if (*field == "iterations") {
            values = test_config.child("attributes").child("iterations");
            for (pugi::xml_node val = values.first_child(); val; val = val.next_sibling())
                iterations.push_back(val.text().as_int());
        } else if (*field == "devices") {
            values = test_config.child("attributes").child("devices");
            for (pugi::xml_node val = values.first_child(); val; val = val.next_sibling())
                devices.push_back(val.text().as_string());
        } else if (*field == "models") {
            values = test_config.child("attributes").child("models");
            for (pugi::xml_node val = values.first_child(); val; val = val.next_sibling()) {
                std::string full_path = val.attribute("full_path").as_string();
                std::string path = val.attribute("path").as_string();
                if (full_path.empty() || path.empty())
                    throw std::logic_error("One of the 'model' records from test config doesn't contain 'full_path' or 'path' attributes");
                else {
                    models.push_back(full_path);
                    models_names.push_back(path);
                }
                std::string precision = val.attribute("precision").as_string();
                precisions.push_back(precision);
            }
        }
    }

    // Initialize variables with default value if it weren't filled
    processes = !processes.empty() ? processes: std::vector<int>{1};
    threads = !threads.empty() ? threads: std::vector<int>{1};
    iterations = !iterations.empty() ? iterations: std::vector<int>{1};
    devices = !devices.empty() ? devices : std::vector<std::string>{"NULL"};
    models = !models.empty() ? models : std::vector<std::string>{"NULL"};
    precisions = !precisions.empty() ? precisions : std::vector<std::string>{"NULL"};
    models_names = !models_names.empty() ? models_names : std::vector<std::string>{"NULL"};

    for (auto &numprocesses : processes)
        for (auto &numthreads : threads)
            for (auto &numiters : iterations)
                for (auto &device : devices)
                    for (int i = 0; i < models.size(); i++)
                        tests_cases.push_back(TestCase(numprocesses, numthreads, numiters, device, models[i], models_names[i], precisions[i]));
    return tests_cases;
}

std::string getTestCaseName(const testing::TestParamInfo<TestCase> &obj) {
    return obj.param.test_case_name;
}

void test_wrapper(const std::function<void(std::string, std::string, int)> &tests_pipeline, const TestCase &params) {
    tests_pipeline(params.model, params.device, params.numiters);
}

void _runTest(const std::function<void(std::string, std::string, int)> &tests_pipeline, const TestCase &params) {
    run_in_threads(params.numthreads, test_wrapper, tests_pipeline, params);
}

void runTest(const std::function<void(std::string, std::string, int)> &tests_pipeline, const TestCase &params) {
#if DEBUG_MODE
    tests_pipeline(params.model, params.device, params.numiters);
#else
    int status = run_in_processes(params.numprocesses, _runTest, tests_pipeline, params);
    ASSERT_EQ(status, 0) << "Test failed with exitcode " << std::to_string(status);
#endif
}

