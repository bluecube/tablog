#include <string>

#include "util/bit_writer.hpp"

namespace tablog::util {

class StringBitWriter;

namespace detail {
    struct StringBitWriterWrapper {
        StringBitWriter* sbw;

        void operator()(char c);
    };
}

/// Bit writer subclass that stores the data directly to std::string.
/// Note that until `end()` (or `flush()`, but careful there) is called,
/// then some data will typically be kept in the buffer of BitWriter.
class StringBitWriter : public BitWriter<detail::StringBitWriterWrapper> {
public:
    StringBitWriter() : BitWriter(detail::StringBitWriterWrapper{this}) {}

    std::string data;
};

inline void detail::StringBitWriterWrapper::operator()(char c) {
    sbw->data.push_back(c);
}

}
