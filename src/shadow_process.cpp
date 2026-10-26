#include "th08/shadow_process.hpp"
#include "modern/headless/session.hpp"
#include "th08/shadow_native.hpp"
#include <cerrno>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

namespace th08::shadow {
namespace {
using Clock = std::chrono::steady_clock;
void transfer(int fd, void *data, std::size_t length, bool writing, unsigned timeout_ms) {
    const auto deadline = Clock::now() + std::chrono::milliseconds(timeout_ms);
    auto *bytes = static_cast<unsigned char *>(data);
    while (length) {
        const auto left =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
        if (left <= 0)
            throw std::runtime_error("shadow_transport_timeout");
        pollfd request{fd, short(writing ? POLLOUT : POLLIN), 0};
        const int ready = poll(&request, 1, int(left));
        if (ready < 0 && errno == EINTR)
            continue;
        if (ready == 0)
            throw std::runtime_error("shadow_transport_timeout");
        if (ready < 0)
            throw std::runtime_error("shadow_transport_poll_failed");
        const auto count =
            writing ? send(fd, bytes, length, MSG_NOSIGNAL) : recv(fd, bytes, length, 0);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            throw std::runtime_error("shadow_transport_closed");
        bytes += count;
        length -= std::size_t(count);
    }
}
void send_packet(int fd, const Packet &packet, unsigned timeout, bool allow_bombs = false) {
    auto bytes = encode(packet, allow_bombs);
    transfer(fd, bytes.data(), bytes.size(), true, timeout);
}
Packet receive_packet(int fd, unsigned timeout, bool allow_bombs = false) {
    Bytes bytes{};
    transfer(fd, bytes.data(), bytes.size(), false, timeout);
    return decode(bytes.data(), bytes.size(), allow_bombs);
}
void follow(int fd, const headless::Config &config, const std::filesystem::path &directory,
            unsigned timeout, bool allow_bombs) {
    std::filesystem::current_path(directory);
    headless::Session session(config);
    Packet receipt;
    receipt.state = capture(session);
    send_packet(fd, receipt, timeout, allow_bombs);
    for (;;) {
        const auto request = receive_packet(fd, timeout, allow_bombs);
        if (request.kind == Kind::Stop)
            return;
        if (request.kind != Kind::Advance || request.status ||
            request.sequence != receipt.sequence + 1 || request.epoch != receipt.epoch)
            throw std::runtime_error("invalid_shadow_receipt_order");
        const auto before = session.state();
        if (stage_terminal(config.story, before) || before.player_state == 2)
            throw std::runtime_error("shadow_input_after_terminal");
        session.step(std::uint16_t(request.input));
        advance_receipt(receipt, request.input, capture(session), allow_bombs);
        send_packet(fd, receipt, timeout, allow_bombs);
    }
}
} // namespace
struct Follower::Impl {
    int fd = -1;
    pid_t child = -1;
    unsigned timeout;
    bool allow_bombs;
    std::filesystem::path directory;
    Impl(const headless::Config &config, unsigned limit, bool bombs)
        : timeout(limit), allow_bombs(bombs) {
        if (!config.dat_path || !std::filesystem::path(config.dat_path).is_absolute() || !timeout ||
            timeout > 60000)
            throw std::runtime_error("invalid_shadow_process_configuration");
        unsigned threads = 0;
        for (const auto &entry : std::filesystem::directory_iterator("/proc/self/task")) {
            (void)entry;
            ++threads;
        }
        if (threads != 1)
            throw std::runtime_error("shadow_fork_requires_single_thread");
        char scratch[] = "/tmp/th08-shadow-XXXXXX";
        if (!mkdtemp(scratch))
            throw std::runtime_error("shadow_scratch_failed");
        directory = scratch;
        int sockets[2];
        if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets) != 0) {
            std::filesystem::remove_all(directory);
            throw std::runtime_error("shadow_socket_failed");
        }
        child = fork();
        if (child == 0) {
            close(sockets[0]);
            int code = 0;
            try {
                follow(sockets[1], config, directory, timeout, allow_bombs);
            } catch (...) {
                Packet error;
                error.kind = Kind::Failure;
                error.status = 1;
                try {
                    send_packet(sockets[1], error, timeout);
                } catch (...) {
                }
                code = 1;
            }
            close(sockets[1]);
            _exit(code);
        }
        close(sockets[1]);
        if (child < 0) {
            close(sockets[0]);
            std::filesystem::remove_all(directory);
            throw std::runtime_error("shadow_fork_failed");
        }
        fd = sockets[0];
    }
    ~Impl() {
        // No unbounded join: this object owns exactly this child and directory.
        if (fd >= 0)
            close(fd);
        if (child > 0) {
            kill(child, SIGTERM);
            int status = 0;
            while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
            }
        }
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
};
Follower::Follower(const headless::Config &config, unsigned timeout, bool allow_bombs)
    : impl_(new Impl(config, timeout, allow_bombs)) {}
Follower::~Follower() = default;
Packet Follower::initial() {
    return receive_packet(impl_->fd, impl_->timeout, impl_->allow_bombs);
}
Packet Follower::advance(std::uint64_t sequence, std::uint64_t epoch, std::uint32_t input) {
    Packet request;
    request.kind = Kind::Advance;
    request.sequence = sequence;
    request.epoch = epoch;
    request.input = input;
    send_packet(impl_->fd, request, impl_->timeout, impl_->allow_bombs);
    return receive_packet(impl_->fd, impl_->timeout, impl_->allow_bombs);
}
} // namespace th08::shadow
