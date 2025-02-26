/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <time.h>

module z0.Log;

import z0.Application;
import z0.Tools;

namespace z0 {

    void Log::open(const shared_ptr<Log>&log) {
        const auto& appConfig = app().getConfig();
        loggingStreams = log;
        if (appConfig.loggingMode & LOGGING_MODE_FILE) {
            loggingStreams->_logFile = fopen("log.txt", "w");
            if(loggingStreams->_logFile == nullptr) {
                die("Error opening log file");
            }
        }
        _LOG("START OF LOG");
    }

    void Log::close() {
        _LOG("END OF LOG");
        const auto& appConfig = app().getConfig();
        if (appConfig.loggingMode & LOGGING_MODE_FILE) {
            fclose(loggingStreams->_logFile);
        }
        loggingStreams.reset();
    }

#ifndef DISABLE_LOG

    streamsize LogStreamBuf::xsputn(const char* s, const streamsize n) {
        const string message(s, n);
        log(message);
        return n;
    }

    streambuf::int_type LogStreamBuf::overflow(const int_type c) {
        if (c != EOF) {
            const char ch = static_cast<char>(c);
            log(string(1, ch));
        }
        return c;
    }

    LogStream::LogStream(const LogLevel level) : ostream{&logStreamBuf} { logStreamBuf.setLevel(level); }

    void LogStreamBuf::log(const string& message) {
        const auto& config = app().getConfig();
        if (config.loggingMode == LOGGING_MODE_NONE || config.logLevelMin > level) {
            return;
        }
        if (newLine) {
            using namespace chrono;
            const auto in_time_t = system_clock::to_time_t(system_clock::now());
            tm tm;
            localtime_s(&tm, &in_time_t);
            string item = wstring_to_string(format(L"{:02}:{:02}:{:02}", tm.tm_hour, tm.tm_min, tm.tm_sec));
            item.append(" ");
            switch (level) {
                case LogLevel::TRACE:     item.append("TRACE"); break;
                case LogLevel::DEBUG:     item.append("DEBUG"); break;
                case LogLevel::INFO:      item.append("INFO "); break;
                case LogLevel::GAME1:     item.append("GAME1"); break;
                case LogLevel::GAME2:     item.append("GAME2"); break;
                case LogLevel::GAME3:     item.append("GAME3"); break;
                case LogLevel::WARNING:   item.append("WARN "); break;
                case LogLevel::ERROR:     item.append("ERROR"); break;
                case LogLevel::CRITICAL:  item.append("CRIT "); break;
                case LogLevel::INTERNAL:  item.append("====="); break;
            }
            item.append(" ");
            if (config.loggingMode & LOGGING_MODE_STDOUT) {
                cout << item;
            }
            if (config.loggingMode & LOGGING_MODE_FILE) {
                fwrite(item.c_str(), item.size(), 1, Log::loggingStreams->_logFile);
            }
        }
        newLine = message == "\n";
        if (config.loggingMode & LOGGING_MODE_STDOUT) {
            cout << message;
            if (newLine) { cout.flush(); }
        }
        if (config.loggingMode & LOGGING_MODE_FILE) {
            fwrite(message.c_str(), message.size(), 1, Log::loggingStreams->_logFile);
            if (newLine) { fflush(Log::loggingStreams->_logFile); }
        }
    }

#endif
}
