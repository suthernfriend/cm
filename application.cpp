#include "application.h"
#include "constants.h"

cm::application::application(std::shared_ptr<cm::config_map> map, std::shared_ptr<cm::logger> log)
        : map(map), kill_timer(ios), signal_set(ios), completed_apps(0), log(log), shutdown_running(false) {
    setup_signal_set();
}

void cm::application::run() {

    setup_children();
    set_signal_handler();

    ios.run();
}

void cm::application::kill_timeout_handler(const boost::system::error_code &ec) {
    for (auto &it : children) {
        if (it.second->terminated())
            continue;

        log->err(app_name, "Forcibly terminating app " + it.first);
        it.second->kill();
    }
}

void cm::application::all_down_handler() {
    log->err(app_name, "Shutdown complete");
    signal_set.cancel();
    kill_timer.cancel();
}

void cm::application::shutdown_handler() {

    bool expected = false;
    bool first = shutdown_running.compare_exchange_strong(expected, true);

    log->err(app_name, "Shutdown: total children: " + std::to_string(map->apps.size()) + ", completed: " +
                       std::to_string(completed_apps.load()));

    if (completed_apps.load() == map->apps.size()) {
        log->err(app_name, "All completed");
        all_down_handler();
        return;
    }

    if (first) {

        log->err(app_name, "Shutdown initiated");

        for (auto &it : children) {
            if (it.second->terminated())
                continue;

            log->err(app_name, "Terminating app " + it.first);
            it.second->terminate();
        }

        kill_timer.expires_from_now(map->kill_delay);

        kill_timer.async_wait([this](auto &ec) { kill_timeout_handler(ec); });
    }
}

void cm::application::signal_handler(const boost::system::error_code &ec, int signal_number) {

    auto ss = find_if(signals.begin(), signals.end(), [&](auto &s) { return s.first == signal_number; });

    if (ss == signals.end())
        log->err(app_name, "Handling signal with number " + std::to_string(signal_number) + " (unknown)");
    else
        log->err(app_name, "Handling signal with number " + std::to_string(signal_number) + " (" + (*ss).second + ")");

    switch (signal_number) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            shutdown_handler();
            break;
        default:
            break;
    }

    if (!shutdown_running.load())
        set_signal_handler();
}

void cm::application::set_signal_handler() {

    log->err(app_name, "Setting signal handler");

    signal_set.async_wait([this](auto &ec, int signo) { signal_handler(ec, signo); });
}

void cm::application::setup_children() {

    log->err(app_name, "Starting applications");

    for (const config_map::configured_application &app : map->apps) {

        try {

            boost::filesystem::path path;

            if (boost::filesystem::exists(app.executable))
                path = app.executable;
            else
                path = bp::search_path(app.executable);

            std::unique_ptr<child> a = std::make_unique<child>(
                    app.name, path, app.args,
                    boost::filesystem::canonical(app.context), app.env, app.term_signal,
                    ios, proc_group
            );

            a->set_on_stdout([this, &app](const child::stream_content_type &buf) {
                linebuffer.available(app.name + "_out", buf, [this, app](const std::string &line) {
                    log->out(app.name, line);
                });
            });

            a->set_on_stderr([this, &app](const child::stream_content_type &buf) {
                linebuffer.available(app.name + "_err", buf, [this, app](const std::string &line) {
                    log->err(app.name, line);
                });
            });

            a->set_on_exit([this, &app](const int exit_code, const std::error_code &code) {
                log->err(app_name, "Application " + app.name + " exited with code " + std::to_string(exit_code) + ".");
                completed_apps++;
                if (app.fail_on_exit) {
                    shutdown_handler();
                } else {
                    if (exit_code != 0) {
                        if (app.fail_on_nonzero_exit)
                            shutdown_handler();
                    }
                }

            });

            children[app.name] = std::move(a);

        } catch (bp::process_error &e) {
            log->err(app_name,
                     "Failed to start process: " + app.name + ": " + e.what() + ". executable is: " + app.executable);
            throw std::runtime_error(e.what());
        }
    }
}

void cm::application::setup_signal_set() {

    log->err(app_name, "Setting signal handlers");
    for (const auto &signal : signals) {
        try {
            signal_set.add(signal.first);
        } catch (const std::runtime_error &e) {
            std::cout << signal.first << signal.second << e.what() << "\n";
            log->err(app_name, e.what());
            return;
        }
    }
}
