#include <iostream>
#include <boost/program_options.hpp>
#include "config_map.h"
#include "application.h"
#include "constants.h"

void show_version() {
    std::cout << cm::app_name << " " << cm::app_version << "\n";
}

void show_help(const std::string &program_name) {
    std::cout << "Usage: " << program_name << " [OPTION]... CONFIG-FILE\n"
              << "  or:  " << program_name << " -e [OPTION]...\n"
              << "Options:\n"
              << "  -j             Use json for log output\n"
              << "  -s             Simple log output\n"
              << "  -e             Load configuration from environment variables\n"
              << "  -h, --help     Show this help\n"
              << "  -v, --version  Show version information\n\n";
}

int main(int argc, char *argv[]) {

    bool use_json = false;
    bool use_env = false;
    std::string config_file;

    for (int i = 1; i < argc; i++) {
        std::string v = argv[i];
        if (v == "-h" || v == "--help") {
            show_help(argv[0]);
            return 0;
        } else if (v == "-v" || v == "--version") {
            show_version();
            return 0;
        } else if (v == "-s") {
            use_json = false;
        } else if (v == "-j") {
            use_json = true;
        } else if (v == "-e") {
            use_env = true;
        } else if (config_file.empty()) {
            config_file = v;
        } else {
            std::cerr << "Invalid option: " << v << "\n";
        }
    }

    try {
        std::shared_ptr<cm::logger> log;

        if (use_json)
            log = std::make_shared<cm::json_logger>(std::cout);
        else
            log = std::make_shared<cm::simple_logger>();

        std::shared_ptr<cm::config_map> config;

        if (use_env)
            config = cm::config_map::from_environment();
        else
            config = cm::config_map::from_file(config_file);

        cm::application app(config, log);

        app.run();
        return 0;

    } catch (const cm::config_map_exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (const std::runtime_error &e) {
        std::cerr << "system error: " << e.what() << "\n";
        return 2;
    }
}
