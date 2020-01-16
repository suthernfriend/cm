//
// Created by jp on 1/14/20.
//

#ifndef CM_LOGGER_H
#define CM_LOGGER_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <ostream>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace cm {

    class logger {

    public:
        enum class stream {
            STDOUT, STDERR
        };

        static std::string stream_to_string(const logger::stream s) {
            switch (s) {
                case logger::stream::STDERR:
                    return "stderr";
                case logger::stream::STDOUT:
                    return "stdout";
                default:
                    return "invalid";
            }
        }

        virtual void log(stream s, const std::string &context, const std::string &line) const = 0;

        void err(const std::string &context, const std::string &line) {
            log(stream::STDERR, context, line);
        }

        void out(const std::string &context, const std::string &line) {
            log(stream::STDOUT, context, line);
        }

    };


    class json_logger : public logger {

    public:
        explicit json_logger(std::ostream &out) : out(out) {
        }

        void log(logger::stream s, const std::string &context, const std::string &line) const override {
            boost::property_tree::ptree tree;

            tree.put("stream", logger::stream_to_string(s));
            tree.put("context", context);
            tree.put("message", line);
            tree.put("time",
                     boost::posix_time::to_iso_extended_string(boost::posix_time::microsec_clock::local_time()));

            std::stringstream ss;
            boost::property_tree::json_parser::write_json(ss, tree, false);

            out << ss.str() << std::flush;
        }

    private:
        std::ostream &out;

    };

    class simple_logger : public logger {

    public:

        void log(stream s, const std::string &context, const std::string &line) const override {
            std::string time = boost::posix_time::to_iso_extended_string(
                    boost::posix_time::microsec_clock::local_time()
            );
            if (s == logger::stream::STDOUT)
                std::cout << "[" << time << "][" << context << "] " << line << "\n" << std::flush;
            else if (s == logger::stream::STDERR)
                std::cerr << "[" << time << "][" << context << "] " << line << "\n" << std::flush;
            else
                abort();
        }

    };

};

#endif //CM_LOGGER_H
