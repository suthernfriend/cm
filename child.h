#ifndef CM_CHILD_H
#define CM_CHILD_H

#include <array>
#include <vector>
#include <functional>
#include <atomic>
#include <system_error>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include "line_buffer.h"

namespace fs = boost::filesystem;
namespace asio = boost::asio;
namespace bp = boost::process;

namespace cm {
    class child {
    public:

        child(const child &) = delete;

        typedef std::array<char, 4096> buffer_type;
        typedef std::vector<char> stream_content_type;
        typedef std::function<void(const stream_content_type &)> read_callback_type;
        typedef std::function<void(const int, const std::error_code &)> exit_callback_type;

        child(std::string name, const fs::path &executable, const std::vector<std::string> &args,
              const fs::path &context,
              const std::map<std::string, std::string> &env, int term_signal,
              asio::io_service &ios, bp::group &group);

        bool terminated();

        void terminate();

        void kill();

        void set_on_exit(const exit_callback_type &t);

        void set_on_stdout(const read_callback_type &t);

        void set_on_stderr(const read_callback_type &t);

    private:
        void on_exit_handler(const int exit, const std::error_code &ec);

        void listen_stdout();

        void listen_stderr();

    private:
        std::string name;
        bp::async_pipe out_pipe, err_pipe, in_pipe;
        bp::child child_process{};

        std::atomic_bool exited;

        int term_signal;
        read_callback_type out_cb, err_cb;
        exit_callback_type exit_cb;
        buffer_type out_buf{}, err_buf{};
    };

    typedef line_buffer<child::stream_content_type> application_line_buffer;

}


#endif //CM_CHILD_H
