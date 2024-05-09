#pragma once

#include "z0/constants.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <string_view>

namespace z0 {

    static constexpr auto& log = cout;

    void die(convertible_to<string_view> auto&& ...s) {
        stringstream stringstream;
        for (auto v : initializer_list<string_view>{ s... }) {
            stringstream << v << " ";
        }
#ifdef _WIN32
        MessageBox(nullptr,
                   stringstream.str().c_str(),
                   ENGINE_NAME.c_str(),
                   MB_OK | MB_ICONINFORMATION);
#endif
        throw runtime_error(stringstream.str());
    }


    vector<string_view> split(string_view str, char delimiter);

}
