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
        LogStream trace{LogLevel::TRACE};
        LogStream _internal{LogLevel::INTERNAL};
        LogStream debug{LogLevel::DEBUG};
        LogStream info{LogLevel::INFO};
        LogStream game1{LogLevel::GAME1};
        LogStream game2{LogLevel::GAME2};
        LogStream game3{LogLevel::GAME3};
        LogStream warning{LogLevel::WARNING};
        LogStream error{LogLevel::ERROR};
        LogStream critical{LogLevel::CRITICAL};

        FILE* _logFile;

        static void open(const shared_ptr<Log>&);
        static void close();
        static inline shared_ptr<Log> loggingStreams{nullptr};
    };


    consteval bool isLoggingEnabled() {
        return ENABLE_LOG;
    }

    template <typename... Args>
    void _LOG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->_internal << ... << args) << endl; } }

    inline void TRACE(const source_location& location = source_location::current()) {
        if constexpr (isLoggingEnabled()) {
            Log::loggingStreams->trace << location.function_name() << " line " << location.line() << endl;
        }
    }

    template <typename... Args>
    void DEBUG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->debug << ... << args) << endl; } }

    template <typename... Args>
    void INFO(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->info << ... << args) << endl; } }

    template <typename... Args>
    void GAME1(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game1 << ... << args) << endl; } }

    template <typename... Args>
    void GAME2(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game2 << ... << args) << endl; } }

    template <typename... Args>
    void GAME3(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game3 << ... << args) << endl; } }

    template <typename... Args>
    void WARNING(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->warning << ... << args) << endl; } }

    template <typename... Args>
    void ERROR(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->error << ... << args) << endl; } }

    template <typename... Args>
    void CRITICAL(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->critical << ... << args) << endl; } }

}
