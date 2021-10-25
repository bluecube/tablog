#include "dynamic/tablog.hpp"

#include <stdexcept>

namespace tablog::dynamic {

Tablog::Tablog(
    dynamic::Encoder encoder,
    std::function<dynamic::Predictor(const std::string& t)> predictorFactory,
    std::vector<std::pair<std::string, std::string>> fieldDescriptors
)
  : encoder(std::move(encoder)) {

    this->encoder->header(formatVersion, fieldDescriptors.size());

    // All value encoders get a fresh instance of the predictor.
    for (const auto& d: fieldDescriptors) {
        auto predictor = predictorFactory(d.second);
        this->encoder->field_header(
            d.first.c_str(),
            predictor->t_is_signed(),
            predictor->sizeof_t()
        );
        valueCompressors.emplace_back(std::move(predictor));
    }

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
