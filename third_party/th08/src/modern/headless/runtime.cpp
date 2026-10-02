#include "runtime.hpp"
#include "modern/windows_runtime.hpp"

uint16_t th08_headless_input = 0;
uint64_t th08_headless_frame = 0;
uint64_t th08::headless::file_io_ns = 0;

namespace th08::modern {
// WinMain is linked for legacy callers, but headless startup bypasses its window
// and file-writing paths. Calling that entry point is unsupported in this build.
bool ConfigureDataDirectory() {
    return false;
}
void InstallCrashReporter() {}
void LogArchiveRequest(const char *) {}
} // namespace th08::modern
