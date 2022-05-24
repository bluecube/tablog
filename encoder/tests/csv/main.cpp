#include "../common.hpp"

#include "tablog.hpp"

#include <string_view>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>

std::string_view trim_whitespace(std::string_view s) {
    constexpr std::string_view whitespace = " 	\r\n";

    const auto start = s.find_first_not_of(whitespace);

    if (start == std::string_view::npos)
        return s.substr(0, 0);

    const auto end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

std::vector<std::string_view> tokenize_csv_line(std::string_view line) {

    std::vector<std::string_view> ret;

    while (const auto token = next_token(line))
        ret.push_back(trim_whitespace(*token));

    return ret;
}

struct StreamOutput {
    void operator()(uint8_t out) {
        stream.put(out);
    }

    std::ostream& stream;
};

class CsvRowEncoderInterface
{
public:
    virtual ~CsvRowEncoderInterface() {}
    virtual void encode(const std::vector<std::string_view>& row) = 0;
    virtual void end_block() = 0;
};

template <typename... ValueTs>
class TablogCsvRowEncoder : public CsvRowEncoderInterface {
public:
    template <typename... EncoderArgs>
    TablogCsvRowEncoder(StreamOutput output, const std::vector<std::string_view>& columnNames)
      : t(std::move(output)) {

        for (std::size_t i = 0; i < columnNames.size(); ++i)
            t.set_column_name(i, columnNames[i]);
    }

    void encode(const std::vector<std::string_view>& row) override {
        if (row.size() != sizeof...(ValueTs))
            throw std::runtime_error("Wrong number of records on row"); // GCOV_EXCL_LINE

        encode(row, std::index_sequence_for<ValueTs...>{});
    }

    void end_block() override {
        t.end_block();
    }

protected:
    template <std::size_t... Indices>
    void encode(const std::vector<std::string_view>& row, std::index_sequence<Indices...>) {
        t.write_row(parse_num<ValueTs>(row[Indices])...);
    }

    tablog::Tablog<StreamOutput, ValueTs...> t;
};


#define FACTORY_ENTRY(types...) \
    { \
        std::vector<std::string_view>(type_names<types>.begin(), type_names<types>.end()), \
        [](auto output, const auto& columnNames) { \
            return std::make_unique<TablogCsvRowEncoder<types>>(std::move(output), columnNames); \
        } \
    }

const std::map<
    std::vector<std::string_view>, // Value types
    std::function<
        std::unique_ptr<CsvRowEncoderInterface>(StreamOutput output, const std::vector<std::string_view>&)
    > // Factory function
> factories = {
    FACTORY_ENTRY(int8_t),
    FACTORY_ENTRY(uint8_t),
    FACTORY_ENTRY(int16_t),
    FACTORY_ENTRY(uint16_t),
    FACTORY_ENTRY(int32_t),
    FACTORY_ENTRY(uint32_t),
    FACTORY_ENTRY(int64_t),
    FACTORY_ENTRY(uint64_t),
    FACTORY_ENTRY(int64_t, uint32_t, uint16_t, uint16_t, uint16_t, uint32_t), // dataset1/tph.csv
    FACTORY_ENTRY(int64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t), // dataset1/power.csv
    FACTORY_ENTRY(int64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t), //dataset1/soc.csv
    FACTORY_ENTRY(uint32_t, uint32_t, uint32_t, int64_t), // gcode/parts.csv
    FACTORY_ENTRY(int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t), // phone_imu/imu.csv
    FACTORY_ENTRY(uint32_t, int16_t, int16_t, int16_t), // phone_imu/magnetometer.csv
    FACTORY_ENTRY(uint32_t, int16_t, uint32_t), // twinsen/*.csv
};


int main() {
    std::string columnLabelsStr;
    std::getline(std::cin, columnLabelsStr);
    const auto columnLabels = tokenize_csv_line(columnLabelsStr);
    std::string typesStr;
    std::getline(std::cin, typesStr);
    const auto types = tokenize_csv_line(typesStr);

    if (columnLabels.size() != types.size())
        throw std::runtime_error("Different number of column labels and column types"); // GCOV_EXCL_LINE

    const auto factoryIt = factories.find(types);
    if (factoryIt == factories.end())
        return 128;

    auto tablogInterface = factoryIt->second(StreamOutput{std::cout}, columnLabels);

    std::string rowStr;
    while(std::getline(std::cin, rowStr)) {
        const auto row = tokenize_csv_line(rowStr);
        tablogInterface->encode(row);
    }

    tablogInterface->end_block();
}
