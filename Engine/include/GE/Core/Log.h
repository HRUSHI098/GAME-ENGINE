#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/ostr.h>

namespace GE {

class Log {
public:
    static void Init();

    static std::shared_ptr<spdlog::logger>& GetCoreLogger()   { return s_CoreLogger; }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

} // namespace GE

// ─── Core engine logging macros ──────────────────────────────────────────────
#define GE_CORE_TRACE(...)  ::GE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...)   ::GE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...)   ::GE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...)  ::GE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_FATAL(...)  ::GE::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ─── Client (game) logging macros ────────────────────────────────────────────
#define GE_TRACE(...)  ::GE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...)   ::GE::Log::GetClientLogger()->info(__VA_ARGS__)
#define GE_WARN(...)   ::GE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...)  ::GE::Log::GetClientLogger()->error(__VA_ARGS__)
#define GE_FATAL(...)  ::GE::Log::GetClientLogger()->critical(__VA_ARGS__)
