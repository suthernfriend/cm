
#ifndef CM_CONSTANTS_H
#define CM_CONSTANTS_H

#include <string>
#include <vector>
#include <csignal>

namespace cm {
    const std::string app_name = "cm";
    const std::string app_version = "1.1.1";
    const std::vector<std::pair<int, std::string>> signals = {

            {SIGINT,    "SIGINT"},
            {SIGILL,    "SIGILL"},
            {SIGABRT,   "SIGABRT"},
            {SIGFPE,    "SIGFPE"},
            {SIGSEGV,   "SIGSEGV"},
            {SIGTERM,   "SIGTERM"},
            {SIGHUP,    "SIGHUP"},
            {SIGQUIT,   "SIGQUIT"},
            {SIGTRAP,   "SIGTRAP"},
            //{SIGKILL,   "SIGKILL"},
            {SIGBUS,    "SIGBUS"},
            {SIGSYS,    "SIGSYS"},
            {SIGPIPE,   "SIGPIPE"},
            {SIGALRM,   "SIGALRM"},
            {SIGURG,    "SIGURG"},
            //{SIGSTOP,   "SIGSTOP"},
            {SIGTSTP,   "SIGTSTP"},
            {SIGCONT,   "SIGCONT"},
            //{SIGCHLD,   "SIGCHLD"},
            {SIGTTIN,   "SIGTTIN"},
            {SIGTTOU,   "SIGTTOU"},
            {SIGPOLL,   "SIGPOLL"},
            {SIGXCPU,   "SIGXCPU"},
            {SIGXFSZ,   "SIGXFSZ"},
            {SIGVTALRM, "SIGVTALRM"},
            {SIGPROF,   "SIGPROF"},
            {SIGUSR1,   "SIGUSR1"},
            {SIGUSR2,   "SIGUSR2"},
            {SIGWINCH,  "SIGWINCH"},
            {SIGIO,     "SIGIO"},
            {SIGIOT,    "SIGIOT"},
#ifdef SIGCLD
            {SIGCLD,    "SIGCLD"},
#endif
    };

    inline int signal_string_to_int(const std::string &signal_str) {
        auto upper = boost::to_upper_copy(signal_str);

        auto it = find_if(signals.begin(), signals.end(), [&](const auto &signal) {
            return upper == std::to_string(signal.first) || upper == signal.second;
        });

        if (it == signals.end())
            throw config_map_exception("Invalid signal " + signal_str);
        else
            return (*it).first;
    }
}

#endif //CM_CONSTANTS_H
