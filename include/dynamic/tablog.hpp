#pragma once

#include "value_compressor.hpp"
#include "dynamic/encoder.hpp"
#include "dynamic/predictor.hpp"

#include <cstdint>
#include <vector>
#include <memory>

namespace tablog::dynamic {

/// Dynamic and runtime configurable tablog verison. Generates identical output
/// to the base Tablog class, but performs dynamic allocations and only supports
/// int64_t inputs.
class Tablog {
public:
    using ValueType = int64_t;
    static constexpr uint_fast16_t formatVersion = 1;

    Tablog(dynamic::Encoder encoder, const dynamic::PredictorInterface& predictor, uint32_t valueCount);
    ~Tablog() { close(); }

    void write(const std::vector<ValueType>& values);
    void close();
private:
    dynamic::Encoder encoder;
    std::vector<detail::ValueCompressor<ValueType, dynamic::Predictor>> valueCompressors;
};

}
