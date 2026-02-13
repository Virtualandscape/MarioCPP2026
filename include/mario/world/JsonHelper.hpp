#pragma once

#include <fstream>
#include <string>
#include <string_view>

namespace mario::JsonHelper {
    // Attempts to open a level file by searching several relative locations.
    // Returns an ifstream to the first found file, or an empty stream if not found.
    std::ifstream open_level_file(std::string_view path);

    // Extracts a string field from a JSON-like text by key.
    // Returns true if the field is found and sets 'value'.
    bool extract_string_field(const std::string &text, std::string_view key, std::string &value);

    // Extracts a float field from a JSON-like text by key.
    // Returns true if the field is found and sets 'value'.
    bool extract_float_field(const std::string &text, std::string_view key, float &value);

    // Extracts a bool field from a JSON-like text by key.
    // Returns true if the field is found and sets 'value'.
    bool extract_bool_field(const std::string &text, std::string_view key, bool &value);
}

