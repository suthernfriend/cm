#ifndef CM_APPLICATION_H
#define CM_APPLICATION_H

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include "logger.h"
#include "child.h"
#include "config_map.h"

namespace cm {

    class application {

    public:
        explicit application(std::shared_ptr<config_map> map, std::shared_ptr<logger> log);

        void run();

    private:

        std::shared_ptr<logger> log;
        std::shared_ptr<config_map> map;
        boost::process::group proc_group;
        boost::asio::io_service ios;
        std::map<std::string, std::unique_ptr<child>> children;
        application_line_buffer linebuffer;
        boost::asio::deadline_timer kill_timer;
        boost::asio::signal_set signal_set;
        int total_apps;
        std::atomic_int completed_apps;
        std::atomic_bool shutdown_running;

        void kill_timeout_handler(const boost::system::error_code &ec);;

        void all_down_handler();

        void shutdown_handler();

        void signal_handler(const boost::system::error_code &ec, int signal_number);

        void setup_signal_set();

        void set_signal_handler();

        void setup_children();

    };
}

#endif //CM_APPLICATION_H
