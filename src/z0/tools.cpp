#include "z0/tools.h"

namespace z0 {

    uint32_t randomi(uint32_t max) {
        static std::random_device rd;
        static std::uniform_int_distribution<> distr(0, static_cast<int>(max));

        std::mt19937 gen(rd());
        return static_cast<uint32_t>(distr(gen));
    }

    float randomf(float max) {
        static std::random_device rd;
        static std::uniform_real_distribution<> distr(0, max);

        std::mt19937 gen(rd());
        return static_cast<float>(distr(gen));
    }

    vector<string_view> split(string_view str, char delimiter) {
        vector<std::string_view> result;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string_view::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }

        result.push_back(str.substr(start)); // Add the last token
        return result;
    }

    string getCurrentDateTime() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto in_time_t = system_clock::to_time_t(now);

        tm tm;
        localtime_s(&tm, &in_time_t);

        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(format(L"{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                        tm.tm_hour, tm.tm_min, tm.tm_sec));
    }


}
