#include "application.h"
#include "constants.h"

cm::application::application(cm::config_map map)
        : map(std::move(map)), kill_timer(ios), signal_set(ios), completed_apps(0) {
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

        log("Forcibly terminating app " + it.first);
        it.second->kill();
    }
}

void cm::application::log(const std::string &app, std::ostream &out, const std::string &line) {
    out << "[" << app << "] " << line << "\n";
}

void cm::application::all_down_handler() {
    log("Shutdown complete");
    signal_set.cancel();
    kill_timer.cancel();
}

void cm::application::shutdown_handler() {

    completed_apps++;

    log("Shutdown: total children: " + std::to_string(map.apps.size()) + ", completed: " +
        std::to_string(completed_apps.load()));

    if (completed_apps.load() == map.apps.size()) {
        log("All completed");
        all_down_handler();
        return;
    }

    if (shutdown_running)
        return;

    shutdown_running = true;

    log("Shutdown initiated");

    for (auto &it : children) {
        if (it.second->terminated())
            continue;

        log("Terminating app " + it.first);
        it.second->terminate();
    }

    kill_timer.expires_from_now(map.kill_delay);

    kill_timer.async_wait([this](auto &ec) { kill_timeout_handler(ec); });
}

void cm::application::signal_handler(const boost::system::error_code &ec, int signal_number) {

    auto ss = find_if(signals.begin(), signals.end(), [&](auto &s) { return s.first == signal_number; });

    log("Handling signal with number " + std::to_string(signal_number) + " (" + (*ss).second + ")");

    switch (signal_number) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            shutdown_handler();
            break;
        default:
            break;
    }

    set_signal_handler();
}

void cm::application::set_signal_handler() {

    log("Setting signal handler");

    signal_set.async_wait([this](auto &ec, int signo) { signal_handler(ec, signo); });
}

void cm::application::setup_children() {

    log("Starting applications");

    for (const config_map::configured_application &app : map.apps) {

        std::unique_ptr<child> a = std::make_unique<child>(
                app.name, bp::search_path(app.executable), app.args,
                boost::filesystem::canonical(app.context), app.env, app.term_signal,
                ios, proc_group
        );

        a->set_on_stdout([this, &app](const child::stream_content_type &buf) {
            linebuffer.available(app.name + "_out", buf, [app](const std::string &line) {
                log(app.name, std::cout, line);
            });
        });

        a->set_on_stderr([this, &app](const child::stream_content_type &buf) {
            linebuffer.available(app.name + "_err", buf, [app](const std::string &line) {
                log(app.name, std::cerr, line);
            });
        });

        a->set_on_exit([this, &app](const int exit_code, const std::error_code &code) {
            log("Application " + app.name + " exited with code " + std::to_string(exit_code) + ".");
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
    }
}

void cm::application::setup_signal_set() {

    log("Setting signal handlers");
    for (const auto &signal : signals) {
        try {
            signal_set.add(signal.first);
        } catch (const std::runtime_error &e) {
            std::cout << signal.first << signal.second << e.what() << "\n";
            log(app_name, std::cerr, e.what());
            return;
        }
    }
}

void cm::application::log(const std::string &app, const std::string &line) {
    log(app_name, std::cout, line);
}

void cm::application::log(const std::string &line) {
    log(app_name, line);
}
