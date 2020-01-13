#ifndef CM_LINE_BUFFER_H
#define CM_LINE_BUFFER_H

#include <functional>
#include <map>

namespace cm {

    template<typename MutableBuffer>
    class line_buffer {

    public:
        typedef std::function<void(std::string)> line_callback_type;
        typedef MutableBuffer buffer_type;

        void available(const std::string &stream_identifier, const buffer_type &new_data, const line_callback_type &cb);

    private:
        std::map<std::string, buffer_type> data;

    };

    template<typename MutableBuffer>
    void line_buffer<MutableBuffer>::available(const std::string &stream_identifier, const buffer_type &new_data,
                                               const line_buffer::line_callback_type &cb) {
        std::copy(new_data.begin(), new_data.end(), std::back_inserter(data[stream_identifier]));
        buffer_type &s = data.at(stream_identifier);

        while (true) {
            auto first_line_break = std::find(s.begin(), s.end(), '\n');

            if (first_line_break == s.end()) {
                return;
            }

            cb(std::string(s.begin(), first_line_break));
            s.erase(s.begin(), first_line_break + 1);
        }
    }
}

#endif //CM_LINE_BUFFER_H
