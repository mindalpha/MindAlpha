//
// Copyright 2021 Mobvista
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include "spdlog/spdlog.h"
#include <cstdlib>
#include <fmt/format.h>
#include <string_view>

namespace mindalpha
{

class LogMessage {
public:
    LogMessage(const char *file, int line, bool is_fatal, spdlog::level::level_enum log_level)
        : is_fatal(is_fatal)
        , log_level(log_level) {
        buf.reserve(1024);
        using namespace std::string_view_literals;
        fmt::format_to(buf, "{}:{} -- "sv, file, line);
    }
    ~LogMessage() {
        spdlog::log(log_level, "{}", str());
        if (is_fatal) {
            abort();
        }
    }
    template <typename T>
    inline LogMessage &operator<<(const T &val) {
        using namespace std::string_view_literals;
        fmt::format_to(buf, "{}"sv, val);
        return *this;
    }
    // for std::endl
    inline LogMessage &operator<<(std::ostream &(*f)(std::ostream &)) {
        buf.push_back('\n');
        return *this;
    }
    inline std::string_view str() const {
        return std::string_view(buf.data(), buf.size());
    }
    inline LogMessage &stream() {
        return *this;
    }

private:
    fmt::memory_buffer buf;
    bool is_fatal = false;
    spdlog::level::level_enum log_level = spdlog::level::info;
};
// Always-on checking
#define CHECK(x)                                                                                   \
    if (!(x))                                                                                      \
    LogMessage(__FILE__, __LINE__, true, spdlog::level::err).stream() << "Check "                  \
                                                                         "failed: " #x             \
                                                                      << ' '
#define CHECK_LT(x, y) CHECK((x) < (y))
#define CHECK_GT(x, y) CHECK((x) > (y))
#define CHECK_LE(x, y) CHECK((x) <= (y))
#define CHECK_GE(x, y) CHECK((x) >= (y))
#define CHECK_EQ(x, y) CHECK((x) == (y))
#define CHECK_NE(x, y) CHECK((x) != (y))
#define CHECK_NOTNULL(x)                                                                           \
    ((x) == NULL ? LogMessage(__FILE__, __LINE__, true, spdlog::level::err).stream()               \
                   << "notnull: " #x << ' ', (x)                                                   \
                 : (x)) // NOLINT(*)

#define LOG_INFO    LogMessage(__FILE__, __LINE__, false, spdlog::level::info)
#define LOG_ERROR   LogMessage(__FILE__, __LINE__, false, spdlog::level::err)
#define LOG_WARNING LogMessage(__FILE__, __LINE__, false, spdlog::level::warn)
#define LOG_FATAL   LogMessage(__FILE__, __LINE__, true, spdlog::level::critical)
#define LOG_QFATAL  LOG_FATAL

// Poor man version of VLOG
#define VLOG(x) LOG_INFO.stream()

#define LOG(severity) LOG_##severity.stream()
#define LG LOG_INFO.stream()
#define LOG_IF(severity, condition) !(condition) ? (void)0 : LOG(severity)

}
