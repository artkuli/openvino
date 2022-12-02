# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

#if (CONAN_EXPORTED)
#    find_package(nlohmann_json)
#else()
    # suppress shadowing names warning
    set(JSON_SystemInclude ON CACHE BOOL "" FORCE)
    add_subdirectory(nlohmann_json EXCLUDE_FROM_ALL)
#endif()
