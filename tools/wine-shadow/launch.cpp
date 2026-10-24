#include "identity.hpp"
#include <cstring>

// Launch only our owned suspended process. Injection failure terminates that
// process before its entry point runs; no existing Wine process is attached.
int main(int argc, char **argv) {
    if (argc != 4 || !wine_shadow::pinned_executable(argv[1])) {
        std::fprintf(stderr, "expected pinned EXE, absolute observer DLL and output directory\n");
        return 2;
    }
    if (!SetEnvironmentVariableA("TH08_SHADOW_DIR", argv[3]))
        return 3;
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    std::string command = std::string("\"") + argv[1] + "\"";
    if (!CreateProcessA(argv[1], command.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr,
                        nullptr, &startup, &process))
        return 3;
    const auto fail = [&](unsigned code) {
        TerminateProcess(process.hProcess, code);
        WaitForSingleObject(process.hProcess, 5000);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return int(code);
    };
    const SIZE_T length = std::strlen(argv[2]) + 1;
    void *remote =
        VirtualAllocEx(process.hProcess, nullptr, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    SIZE_T written = 0;
    if (!remote || !WriteProcessMemory(process.hProcess, remote, argv[2], length, &written) ||
        written != length)
        return fail(4);
    const auto load = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
    HANDLE thread = CreateRemoteThread(process.hProcess, nullptr, 0, load, remote, 0, nullptr);
    DWORD result = 0;
    const bool injected = thread && WaitForSingleObject(thread, 10000) == WAIT_OBJECT_0 &&
                          GetExitCodeThread(thread, &result) && result;
    if (thread)
        CloseHandle(thread);
    if (!injected)
        return fail(5);
    VirtualFreeEx(process.hProcess, remote, 0, MEM_RELEASE);
    // Map the same DLL only to resolve its export RVA; do not run its entry
    // point in the launcher. Actual initialization runs remotely after LoadLibrary.
    const auto local = LoadLibraryExA(argv[2], nullptr, DONT_RESOLVE_DLL_REFERENCES);
    const auto initialize = local ? GetProcAddress(local, "Th08ShadowInitialize") : nullptr;
    if (!initialize) {
        if (local)
            FreeLibrary(local);
        return fail(7);
    }
    const auto offset = reinterpret_cast<unsigned>(initialize) - reinterpret_cast<unsigned>(local);
    FreeLibrary(local);
    thread = CreateRemoteThread(process.hProcess, nullptr, 0,
                                reinterpret_cast<LPTHREAD_START_ROUTINE>(result + offset), nullptr,
                                0, nullptr);
    DWORD initialized = 0;
    const bool ready = thread && WaitForSingleObject(thread, 10000) == WAIT_OBJECT_0 &&
                       GetExitCodeThread(thread, &initialized) && initialized == 1;
    if (thread)
        CloseHandle(thread);
    if (!ready)
        return fail(8);
    std::printf("owned_pid=%lu observer=%lx\n", process.dwProcessId, result);
    std::fflush(stdout);
    if (ResumeThread(process.hThread) == DWORD(-1))
        return fail(6);
    CloseHandle(process.hThread);
    WaitForSingleObject(process.hProcess, INFINITE);
    GetExitCodeProcess(process.hProcess, &result);
    CloseHandle(process.hProcess);
    return int(result);
}
