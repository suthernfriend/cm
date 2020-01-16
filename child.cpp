//
// Created by jp on 1/13/20.
//

#include "child.h"

cm::child::child(std::string name, const fs::path &executable, const std::vector<std::string> &args,
                 const fs::path &context, const std::map<std::string, std::string> &env, int term_signal,
                 asio::io_service &ios, bp::group &group)
        : name(std::move(name)), out_pipe(ios), err_pipe(ios), in_pipe(ios), exited(false),
          term_signal(term_signal) {
    
    auto boost_env = boost::this_process::environment();

    for (const auto &it : env)
        boost_env[it.first] = it.second;

    child_process = bp::child(bp::exe = executable, bp::args = args, bp::start_dir = context,
                              bp::env = bp::environment(boost_env),
                              ios, group,
                              bp::std_in < in_pipe, bp::std_out > out_pipe, bp::std_err > err_pipe,
                              bp::on_exit = [this](const int exit, const std::error_code &ec) {
                                  on_exit_handler(exit, ec);
                              }
    );

    listen_stdout();
    listen_stderr();
}

bool cm::child::terminated() {
    return exited.load();
}

void cm::child::terminate() {
    int pid = child_process.native_handle();
    ::kill(pid, term_signal);
}

void cm::child::kill() {
    int pid = child_process.native_handle();
    ::kill(pid, SIGKILL);
}

void cm::child::set_on_exit(const cm::child::exit_callback_type &t) {
    exit_cb = t;
}

void cm::child::set_on_stdout(const cm::child::read_callback_type &t) {
    out_cb = t;
}

void cm::child::set_on_stderr(const cm::child::read_callback_type &t) {
    err_cb = t;
}

void cm::child::on_exit_handler(const int exit, const std::error_code &ec) {
    exited.store(true);
    exit_cb(exit, ec);
}

void cm::child::listen_stdout() {
    if (!exited.load())
        asio::async_read(out_pipe, asio::buffer(out_buf),
                         boost::asio::transfer_at_least(1),
                         [this](const boost::system::error_code &ec, std::size_t len) {
                             stream_content_type data(len);
                             std::copy(out_buf.begin(), out_buf.begin() + len, data.begin());
                             out_cb(data);

                             listen_stdout();
                         });
}

void cm::child::listen_stderr() {
    if (!exited.load())
        asio::async_read(err_pipe, asio::buffer(err_buf),
                         boost::asio::transfer_at_least(1),
                         [this](const boost::system::error_code &ec, std::size_t len) {
                             std::vector<char> data(len);
                             std::copy(err_buf.begin(), err_buf.begin() + len, data.begin());
                             err_cb(data);

                             listen_stderr();
                         });
}
