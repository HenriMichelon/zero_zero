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

    constexpr bool ENABLE_LOG = false;

    class LogStream {
    public:
        inline LogStream(LogLevel level) {};
    private:
    };

#else

    constexpr bool ENABLE_LOG = true;

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
        static inline LogStream trace{LogLevel::TRACE};
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

    consteval bool isLoggingEnabled() {
        return ENABLE_LOG;
    }

    template <typename... Args>
    void _LOG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::_internal << ... << args) << endl; } }

    inline void TRACE(const source_location& location = source_location::current()) {
        if constexpr (isLoggingEnabled()) {
            Log::trace << location.function_name() << " line " << location.line() << endl;
        }
    }

    template <typename... Args>
    void DEBUG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::debug << ... << args) << endl; } }

    template <typename... Args>
    void INFO(Args... args) { if constexpr (isLoggingEnabled()) { (Log::info << ... << args) << endl; } }

    template <typename... Args>
    void GAME1(Args... args) { if constexpr (isLoggingEnabled()) { (Log::game1 << ... << args) << endl; } }

    template <typename... Args>
    void GAME2(Args... args) { if constexpr (isLoggingEnabled()) { (Log::game2 << ... << args) << endl; } }

    template <typename... Args>
    void GAME3(Args... args) { if constexpr (isLoggingEnabled()) { (Log::game3 << ... << args) << endl; } }

    template <typename... Args>
    void WARNING(Args... args) { if constexpr (isLoggingEnabled()) { (Log::warning << ... << args) << endl; } }

    template <typename... Args>
    void ERROR(Args... args) { if constexpr (isLoggingEnabled()) { (Log::error << ... << args) << endl; } }

    template <typename... Args>
    void CRITICAL(Args... args) { if constexpr (isLoggingEnabled()) { (Log::critical << ... << args) << endl; } }


}
