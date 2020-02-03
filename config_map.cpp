#include <iostream>
#include "config_map.h"
#include "constants.h"

std::shared_ptr<cm::config_map> cm::config_map::from_file(const std::string &file) {


    auto map = std::make_shared<cm::config_map>(10000);

    try {
        auto config = YAML::LoadFile(file);

        if (!config["version"])
            throw config_map_exception("version key is required");
        if (config["version"].as<int>() == 1)
            map->parse_v1(config);
        else
            throw config_map_exception("unknown version");
    } catch (const YAML::Exception &e) {
        throw config_map_exception(e.what());
    }

    return map;
}

cm::config_map::config_map(int kill_delay)
        : kill_delay(kill_delay) {
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
        n.context = "/";
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

void cm::config_map::parse_v1(const YAML::Node &config) {

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

std::shared_ptr<cm::config_map> cm::config_map::from_environment() {
    auto map = std::make_shared<config_map>(10000);

    auto env = boost::this_process::environment();

    if (env.find("CM_VERSION") == env.end())
        throw config_map_exception("environment variable with name CM_VERSION is required");

    auto version = env["CM_VERSION"].to_string();

    if (version == "1") {
        auto node = env_to_yaml_v1(env);
        map->parse_v1(node);
    } else {
        throw config_map_exception("unknown version");
    }

    return map;

}

bool is_equal(
        const std::vector<std::string>::const_iterator &begin,
        const std::vector<std::string>::const_iterator &end,
        const std::initializer_list<std::string> &list) {
    return std::equal(begin, end, list.begin());
}

YAML::Node cm::config_map::env_to_yaml_v1(const boost::process::environment &env) {

    const std::string prefix = "CM";

    YAML::Node root;

    root["apps"] = {};

    for (const auto &entry : env) {
        const std::string key = entry.get_name();

        if (key.find(prefix + "_") == 0) {
            std::vector<std::string> split;
            boost::split(split, key, boost::is_any_of("_"));

            if (is_equal(split.begin(), split.end(), {prefix, "VERSION"}))
                root["version"] = entry.to_string();
            else if (is_equal(split.begin(), split.end(), {prefix, "KILL-DELAY"}))
                root["kill-delay"] = entry.to_string();
            else if (is_equal(split.begin(), split.begin() + 2, {prefix, "APPS"})) {
                const std::string name = boost::to_lower_copy(split.at(2));
                if (!root["apps"][name].IsMap())
                    root["apps"][name] = {};
                const std::string option = split.at(3);

                if (option == "CONTEXT")
                    root["apps"][name]["context"] = entry.to_string();
                else if (option == "EXEC")
                    root["apps"][name]["exec"] = entry.to_string();
                else if (option == "FAIL-ON-EXIT")
                    root["apps"][name]["fail-on-exit"] = entry.to_string();
                else if (option == "TERM-SIGNAL")
                    root["apps"][name]["term-signal"] = entry.to_string();
                else
                    throw config_map_exception("Unknown app option: " + key);
            } else {
                throw config_map_exception("Unknown option: " + key);
            }
        }
    }

    return root;
}

cm::config_map_exception::config_map_exception(std::string s) : message(std::move(s)) {}

const char *cm::config_map_exception::what() const noexcept {
    return message.c_str();
}
