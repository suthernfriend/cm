#include "config_map.h"
#include "constants.h"

cm::config_map::config_map(const std::string &file)
        : kill_delay(10000) {
    try {
        config = YAML::LoadFile(file);

        if (!config["version"])
            throw config_map_exception("version key is required");
        if (config["version"].as<int>() == 1)
            parse_v1();
        else
            throw config_map_exception("unknown version");
    } catch (const YAML::Exception &e) {
        throw config_map_exception(e.what());
    }
}

cm::config_map::configured_application cm::config_map::parse_v1_app(const std::string &name, const YAML::Node &node) {

    configured_application n;

    n.name = name;

    if (!node["exec"])
        throw config_map_exception("app " + name + " requires exec key");
    if (node["exec"].IsScalar()) {
        auto s = node["exec"].as<std::string>();

        boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(
                s, boost::escaped_list_separator<char>('\\', ' ', '\"')
        );

        auto it = tokenizer.begin();
        n.executable = *tokenizer.begin();

        it++;
        for (; it != tokenizer.end(); it++) {
            n.args.push_back(*it);
        }
    }

    if (!node["context"])
        throw config_map_exception("app " + name + " requires context");
    else if (!node["context"].IsScalar())
        throw config_map_exception("app " + name + " context needs to be scalar");
    else
        n.context = node["context"].as<std::string>();

    auto &fail_on_exit_node = node["fail-on-exit"];
    if (fail_on_exit_node && fail_on_exit_node.IsScalar())
        n.fail_on_exit = fail_on_exit_node.as<bool>();
    else
        n.fail_on_exit = true;

    auto &fail_on_non_zero_exit_node = node["fail-on-nonzero-exit"];
    if (fail_on_non_zero_exit_node && fail_on_non_zero_exit_node.IsScalar())
        n.fail_on_nonzero_exit = fail_on_non_zero_exit_node.as<bool>();
    else
        n.fail_on_nonzero_exit = true;

    auto &term_signal = node["term-signal"];
    if (term_signal && term_signal.IsScalar())
        n.term_signal = signal_string_to_int(term_signal.as<std::string>());
    else
        n.term_signal = SIGTERM;

    if (node["env"] && node["env"].IsMap()) {
        for (auto it = node["env"].begin(); it != node["env"].end(); it++) {
            auto k = it->first.as<std::string>();
            auto s = it->second;

            if (!s.IsScalar())
                throw config_map_exception(
                        "app " + name + " has env defintion of " + k + " which is not a scalar.");

            n.env[it->first.as<std::string>()] = it->second.as<std::string>();
        }
    }

    return n;
}

void cm::config_map::parse_v1() {

    auto apps_k = config["apps"];

    if (!apps_k)
        throw config_map_exception("apps key is required");
    if (!apps_k.IsMap())
        throw config_map_exception("apps must be a map");

    auto kill_delay_l = config["kill-delay"];
    if (kill_delay_l && kill_delay_l.IsScalar())
        kill_delay = boost::posix_time::milliseconds(kill_delay_l.as<int>());

    for (YAML::const_iterator it = apps_k.begin(); it != apps_k.end(); it++) {
        if (!it->second.IsMap())
            throw config_map_exception(
                    "child key with name '" + it->first.as<std::string>() + "' must be a map");
        configured_application na = parse_v1_app(it->first.as<std::string>(), it->second);
        apps.push_back(na);
    }
}

cm::config_map_exception::config_map_exception(std::string s) : message(std::move(s)) {}

const char *cm::config_map_exception::what() const noexcept {
    return message.c_str();
}
