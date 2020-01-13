#ifndef CM_CONFIG_MAP_H
#define CM_CONFIG_MAP_H

#include <vector>
#include <string>
#include <csignal>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/tokenizer.hpp>

namespace cm {

    class config_map_exception : std::exception {

        std::string message;

    public:
        explicit config_map_exception(std::string s);

        [[nodiscard]] const char *what() const noexcept override;
    };

    class config_map {
    public:

        struct configured_application {
            std::string name, executable, context;
            std::vector<std::string> args;
            int term_signal;
            std::map<std::string, std::string> env;
            bool fail_on_exit = true;
            bool fail_on_nonzero_exit = true;
        };

        std::vector<configured_application> apps;
        boost::posix_time::milliseconds kill_delay;

        explicit config_map(const std::string &file);

    private:
        static configured_application parse_v1_app(const std::string &name, const YAML::Node &node);

        void parse_v1();

        YAML::Node config;
    };
}

#endif //CM_CONFIG_MAP_H
