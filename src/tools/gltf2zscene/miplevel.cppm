/*
 * Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
import std;
using namespace std;

export module miplevel;

export struct MipLevel {
    uint32_t                    width;
    uint32_t                    height;
    shared_ptr<vector<uint8_t>> data;
};
