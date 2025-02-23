/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Log;

import z0.Constants;

export namespace z0 {

#ifdef DISABLE_LOG
    class LogStream : public ostream {
    public:
        explicit LogStream(LogLevel level) = default;
    private:
    };
#else
    class LogStreamBuf : public streambuf {
    public:
        streamsize xsputn(const char* s, std::streamsize n) override;

        int_type overflow(int_type c) override;

        void setLevel(const LogLevel level) { this->level = level; }

    private:
        LogLevel level{LogLevel::ERROR};
        bool newLine{true};
        void log(const string& message);
    };

    class LogStream : public ostream {
    public:
        explicit LogStream(LogLevel level);
    private:
        LogStreamBuf logStreamBuf;
    };

#endif

    /**
     * Logging streams
     */
    struct Log {
        static inline LogStream _internal{LogLevel::INTERNAL};
        static inline LogStream debug{LogLevel::DEBUG};
        static inline LogStream info{LogLevel::INFO};
        static inline LogStream game1{LogLevel::GAME1};
        static inline LogStream game2{LogLevel::GAME2};
        static inline LogStream game3{LogLevel::GAME3};
        static inline LogStream warning{LogLevel::WARNING};
        static inline LogStream error{LogLevel::ERROR};
        static inline LogStream critical{LogLevel::CRITICAL};
    };

}
