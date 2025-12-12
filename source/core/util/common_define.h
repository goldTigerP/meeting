#pragma once

#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace meeting {
inline int64_t GetCurrentTimeStamp() {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now())
        .time_since_epoch()
        .count();
}

inline std::string GetCurrentTimeStringWithMillis() {
    auto now     = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    auto tm      = *std::localtime(&nowTime);

    // 获取毫秒部分
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":" << std::setw(2) << tm.tm_min
        << ":" << std::setw(2) << tm.tm_sec << "." << std::setw(3) << millis.count();

    return oss.str();
}

namespace deatil {
class Log {
public:
    Log(std::string level, std::string file, int line) : level_(level) {
        stream_ << "[Meeting] " << level << ' ' << GetCurrentTimeStringWithMillis();
        stream_ << ' ' << file << ":" << line << ' ';
    }

    ~Log() {
        Output();
    }

    std::stringstream &Stream() {
        return stream_;
    }

    void Output() {
        stream_ << '\n';
        const auto logStr = stream_.str();

        if (level_ == "ERROR") {
            std::cerr << logStr;
        } else {
            std::cout << logStr;
        }
    }

private:
    std::stringstream stream_{};
    std::string level_;
};

template <typename T>
inline std::stringstream &operator<<(std::stringstream &os, const T &data) {
    return os << std::to_string(data);
}
}  // namespace deatil

}  // namespace meeting

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

// 自定义文件名提取宏（兼容旧版编译器）
#ifdef __FILE_NAME__
    #define FILENAME __FILE_NAME__
#else
    // 提取文件名的辅助函数
    constexpr const char* extract_filename(const char* path) {
        const char* filename = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '/' || *p == '\\') {
                filename = p + 1;
            }
        }
        return filename;
    }
    #define FILENAME extract_filename(__FILE__)
#endif

#define LOG_MESSAGE(level) meeting::deatil::Log(level, FILENAME, __LINE__).Stream()

#define LOG_INFO LOG_MESSAGE("INFO")
#define LOG_WARN LOG_MESSAGE("WARN")
#define LOG_ERROR LOG_MESSAGE("ERROR")
