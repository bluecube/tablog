#include "dynamic/tablog.hpp"

#include <stdexcept>

namespace tablog::dynamic {

Tablog::Tablog(dynamic::Encoder encoder, const dynamic::PredictorInterface& predictor, uint32_t valueCount)
  : encoder(std::move(encoder)) {

    // All value encoders get a fresh instance of the predictor.
    for (size_t i = 0; i < valueCount; ++i)
        valueCompressors.emplace_back(predictor.make_new());

    this->encoder->header(formatVersion, valueCount);
    // must be called through this, because the argument is already moved away from
}

void Tablog::write(const std::vector<ValueType>& values) {
    if (values.size() != valueCompressors.size())
        throw std::runtime_error("Mismatched vector sizes");
    for (size_t i = 0; i < values.size(); ++i)
        valueCompressors[i].write(values[i], encoder);
}

void Tablog::close() {
    if (!encoder)
        return;
    encoder->end_of_stream();
    encoder = nullptr;
}

}
