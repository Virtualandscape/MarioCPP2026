#include "mario/world/JsonHelper.hpp"

#include <filesystem>
#include <cctype>


// Namespace containing small JSON-like helper utilities used by Level and related classes.
// These helpers perform minimal, text-based extraction of simple fields (string/float/bool)
// from a JSON-like file. They are intentionally lightweight to preserve original behavior.
namespace mario::JsonHelper {
    // Attempts to open a level file by trying a few relative paths.
    // Returns a valid ifstream if a file is found, otherwise returns an empty stream.
    std::ifstream open_level_file(std::string_view path) {
        std::filesystem::path base(path);
        std::ifstream file{base.string()};
        if (file) {
            return file;
        }

        // Try a few likely relative locations based on the current working directory.
        const std::filesystem::path cwd = std::filesystem::current_path();
        const std::filesystem::path tries[] = {
            cwd / base,
            cwd / ".." / base,
            cwd / ".." / ".." / base,
            cwd / ".." / ".." / ".." / base,
        };

        for (const auto &candidate: tries) {
            // Attempt to open each candidate path and return the first success.
            file = std::ifstream{candidate.string()};
            if (file) {
                return file;
            }
        }

        // No file found, return empty stream.
        return {};
    }

    // Extracts a quoted string field from 'text' by looking for '"key" : "value"' pattern.
    // Returns true and sets 'value' on success; returns false if the key or value is missing.
    bool extract_string_field(const std::string &text, std::string_view key, std::string &value) {
        const std::string needle = std::string("\"") + std::string(key) + "\"";
        std::size_t pos = text.find(needle);
        if (pos == std::string::npos) {
            return false;
        }

        // Find the colon after the key and skip whitespace.
        pos = text.find(':', pos);
        if (pos == std::string::npos) {
            return false;
        }

        ++pos;
        while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
            ++pos;
        }

        // Expect an opening quote for the string value.
        if (pos >= text.size() || text[pos] != '"') {
            return false;
        }

        // Extract until the closing quote.
        const std::size_t start = pos + 1;
        const std::size_t end = text.find('"', start);
        if (end == std::string::npos) {
            return false;
        }

        value = text.substr(start, end - start);
        return true;
    }

    // Extracts a numeric field (float) from 'text' by locating '"key" : <number>'.
    // Accepts digits, sign, decimal point and exponent notation. Returns true on success.
    bool extract_float_field(const std::string &text, std::string_view key, float &value) {
        const std::string needle = std::string("\"") + std::string(key) + "\"";
        std::size_t pos = text.find(needle);
        if (pos == std::string::npos) {
            return false;
        }

        // Find colon and skip whitespace before the numeric literal.
        pos = text.find(':', pos);
        if (pos == std::string::npos) {
            return false;
        }

        ++pos;
        while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
            ++pos;
        }

        // Parse number characters (0-9, signs, decimal point, exponent).
        const std::size_t start = pos;
        std::size_t end = start;
        while (end < text.size()) {
            const char c = text[end];
            if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') {
                ++end;
                continue;
            }
            break;
        }
        if (end == start) return false;

        try {
            // Convert substring to float; catch conversion errors and return false.
            value = std::stof(text.substr(start, end - start));
        } catch (...) {
            return false;
        }
        return true;
    }

    // Extracts a boolean field by locating '"key" : true' or '"key" : false'.
    // Returns true and sets 'value' if matched; otherwise returns false.
    bool extract_bool_field(const std::string &text, std::string_view key, bool &value) {
        const std::string needle = std::string("\"") + std::string(key) + "\"";
        std::size_t pos = text.find(needle);
        if (pos == std::string::npos) {
            return false;
        }

        // Find colon and skip whitespace, then compare literal text.
        pos = text.find(':', pos);
        if (pos == std::string::npos) {
            return false;
        }

        ++pos;
        while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
            ++pos;
        }

        if (text.substr(pos, 4) == "true") {
            value = true;
            return true;
        } else if (text.substr(pos, 5) == "false") {
            value = false;
            return true;
        }
        return false;
    }

} // namespace mario::JsonHelper

