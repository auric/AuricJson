#pragma once

#include <cctype>
#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

// Helper functions
constexpr bool isspace(char c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

constexpr bool isdigit(char c) {
    return c >= '0' && c <= '9';
}

class JsonParser {
public:
    struct JsonArray;
    struct JsonObject;

    using JsonValue = std::variant<
        std::nullptr_t,
        bool,
        int,
        double,
        std::string_view,
        JsonArray,
        JsonObject
        >;

    struct JsonArray {
        std::vector<JsonValue> elements;

        const JsonValue& operator[](size_t index) const {
            return elements[index];
        }
    };

    struct JsonObject {
        std::vector<std::pair<std::string_view, JsonValue>> members;

        const JsonValue& operator[](std::string_view key) const {
            for (const auto& [k, v] : members) {
                if (k == key) {
                    return v;
                }
            }
            throw std::runtime_error("Key not found: " + std::string(key));
        }
    };

    JsonParser() noexcept = default;
    ~JsonParser() noexcept = default;
    JsonParser(const JsonParser& other) noexcept = default;
    JsonParser& operator=(const JsonParser& other) noexcept = default;
    JsonParser(JsonParser&& other) noexcept = default;
    JsonParser& operator=(JsonParser&& other) noexcept = default;

    JsonValue parse(std::string_view json) const {
        size_t pos = 0;
        skipWhitespace(json, pos);
        return parseValue(json, pos);
    }

    static constexpr bool isNull(const JsonValue& value) {
        return std::holds_alternative<std::nullptr_t>(value);
    }

    static constexpr bool isBool(const JsonValue& value) {
        return std::holds_alternative<bool>(value);
    }

    static constexpr bool isInt(const JsonValue& value) {
        return std::holds_alternative<int>(value);
    }

    static constexpr bool isDouble(const JsonValue& value) {
        return std::holds_alternative<double>(value);
    }

    static constexpr bool isString(const JsonValue& value) {
        return std::holds_alternative<std::string_view>(value);
    }

    static constexpr bool isArray(const JsonValue& value) {
        return std::holds_alternative<JsonArray>(value);
    }

    static constexpr bool isObject(const JsonValue& value) {
        return std::holds_alternative<JsonObject>(value);
    }

    static constexpr bool toBool(const JsonValue& value) {
        if (!isBool(value)) {
            throw std::runtime_error("Value is not a boolean");
        }
        return std::get<bool>(value);
    }

    static int toInt(const JsonValue& value) {
        if (!isInt(value)) {
            throw std::runtime_error("Value is not an integer");
        }
        return std::get<int>(value);
    }

    static constexpr double toDouble(const JsonValue& value) {
        if (!isDouble(value)) {
            throw std::runtime_error("Value is not a double");
        }
        return std::get<double>(value);
    }

    static constexpr std::string_view toString(const JsonValue& value) {
        if (!isString(value)) {
            throw std::runtime_error("Value is not a string");
        }
        return std::get<std::string_view>(value);
    }

    static const JsonArray& toArray(const JsonValue& value) {
        if (!isArray(value)) {
            throw std::runtime_error("Value is not an array");
        }
        return std::get<JsonArray>(value);
    }

    static const JsonObject& toObject(const JsonValue& value) {
        if (!isObject(value)) {
            throw std::runtime_error("Value is not an object");
        }
        return std::get<JsonObject>(value);
    }

private:
    void skipWhitespace(std::string_view json, size_t& pos) const {
        while (pos < json.size() && isspace(json[pos]))
            ++pos;
    }

    constexpr char peek(std::string_view json, size_t pos) const {
        if (pos >= json.size())
            throw std::runtime_error("Unexpected end of JSON");
        return json[pos];
    }

    constexpr char consume(std::string_view json, size_t& pos) const {
        if (pos >= json.size())
            throw std::runtime_error("Unexpected end of JSON");
        return json[pos++];
    }

    constexpr JsonValue parseValue(std::string_view json, size_t& pos) const {
        switch (peek(json, pos)) {
        case 'n': return parseNull(json, pos);
        case 't': return parseTrue(json, pos);
        case 'f': return parseFalse(json, pos);
        case '"': return parseString(json, pos);
        case '[': return parseArray(json, pos);
        case '{': return parseObject(json, pos);
        default: return parseNumber(json, pos);
        }
    }

    constexpr std::nullptr_t parseNull(std::string_view json, size_t& pos) const {
        constexpr auto null = std::string_view("ull");
        if (json.substr(pos + 1, null.size()) == null) {
            pos += 4;
            return nullptr;
        }
        throw std::runtime_error("Invalid JSON: expected 'null'");
    }

    constexpr bool parseTrue(std::string_view json, size_t& pos) const {
        constexpr auto str = std::string_view("rue");
        if (json.substr(pos + 1, str.size()) == str) {
            pos += 4;
            return true;
        }
        throw std::runtime_error("Invalid JSON: expected 'true'");
    }

    constexpr bool parseFalse(std::string_view json, size_t& pos) const {
        constexpr auto str = std::string_view("alse");
        if (json.substr(pos + 1, str.size()) == str) {
            pos += 5;
            return false;
        }
        throw std::runtime_error("Invalid JSON: expected 'false'");
    }

    constexpr std::string_view parseString(std::string_view json, size_t& pos) const {
        consume(json, pos); // consume opening quote
        size_t startPos = pos;
        while (peek(json, pos) != '"') {
            if (peek(json, pos) == '\\') {
                ++pos; // skip escape character
                switch (peek(json, pos)) {
                case '"':
                case '\\':
                case '/':
                    ++pos;
                    break;
                case 'b':
                    ++pos;
                    break;
                case 'f':
                    ++pos;
                    break;
                case 'n':
                    ++pos;
                    break;
                case 'r':
                    ++pos;
                    break;
                case 't':
                    ++pos;
                    break;
                case 'u': {
                    ++pos;
                    uint32_t codepoint = 0;
                    for (int i = 0; i < 4; ++i) {
                        char c = peek(json, pos);
                        if (c >= '0' && c <= '9')
                            codepoint = (codepoint << 4) + (c - '0');
                        else if (c >= 'A' && c <= 'F')
                            codepoint = (codepoint << 4) + (c - 'A' + 10);
                        else if (c >= 'a' && c <= 'f')
                            codepoint = (codepoint << 4) + (c - 'a' + 10);
                        else
                            throw std::runtime_error("Invalid Unicode escape sequence");
                        ++pos;
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Invalid escape sequence");
                }
            } else {
                ++pos;
            }
        }
        std::string_view str = json.substr(startPos, pos - startPos);
        ++pos; // consume closing quote
        return str;
    }

    JsonValue parseNumber(std::string_view json, size_t& pos) const {
        size_t endPos = pos;
        bool isFloatingPoint = false;

        // Check for optional minus sign
        if (endPos < json.size() && json[endPos] == '-')
            ++endPos;

        // Parse integer part
        while (endPos < json.size() && isdigit(json[endPos]))
            ++endPos;

        // Check for optional fractional part
        if (endPos < json.size() && json[endPos] == '.') {
            isFloatingPoint = true;
            ++endPos;
            while (endPos < json.size() && isdigit(json[endPos]))
                ++endPos;
        }

        // Check for optional exponent part
        if (endPos < json.size() && (json[endPos] == 'e' || json[endPos] == 'E')) {
            isFloatingPoint = true;
            ++endPos;
            if (endPos < json.size() && (json[endPos] == '+' || json[endPos] == '-'))
                ++endPos;
            while (endPos < json.size() && isdigit(json[endPos]))
                ++endPos;
        }

        if (isFloatingPoint) {
            // Floating-point number
            double num;
            auto result = std::from_chars(json.data() + pos, json.data() + endPos, num);

            if (result.ec != std::errc()) {
                throw std::runtime_error("Invalid number format");
            }

            pos = endPos;
            return num;
        } else {
            // Integer number
            int num;
            auto result = std::from_chars(json.data() + pos, json.data() + endPos, num);

            if (result.ec != std::errc()) {
                throw std::runtime_error("Invalid number format");
            }

            pos = endPos;
            return num;
        }
    }

    JsonArray parseArray(std::string_view json, size_t& pos) const {
        JsonArray arr;
        consume(json, pos); // consume opening bracket
        skipWhitespace(json, pos);
        if (peek(json, pos) != ']') {
            size_t elemCount = 0;
            while (true) {
                arr.elements.emplace_back(parseValue(json, pos));
                ++elemCount;
                skipWhitespace(json, pos);
                if (peek(json, pos) == ']')
                    break;
                if (consume(json, pos) != ',')
                    throw std::runtime_error("Invalid JSON: expected ',' or ']'");
                skipWhitespace(json, pos);
            }
            arr.elements.reserve(elemCount); // Avoid reallocations
        }
        consume(json, pos); // consume closing bracket
        return arr;
    }

    JsonObject parseObject(std::string_view json, size_t& pos) const {
        JsonObject obj;
        consume(json, pos); // consume opening brace
        skipWhitespace(json, pos);
        if (peek(json, pos) != '}') {
            size_t memberCount = 0;
            while (true) {
                std::string_view key = parseString(json, pos);
                skipWhitespace(json, pos);
                if (consume(json, pos) != ':')
                    throw std::runtime_error("Invalid JSON: expected ':'");
                skipWhitespace(json, pos);
                obj.members.emplace_back(std::string(key), parseValue(json, pos));
                ++memberCount;
                skipWhitespace(json, pos);
                if (peek(json, pos) == '}')
                    break;
                if (consume(json, pos) != ',')
                    throw std::runtime_error("Invalid JSON: expected ',' or '}'");
                skipWhitespace(json, pos);
            }
            obj.members.reserve(memberCount); // Avoid reallocations
        }
        consume(json, pos); // consume closing brace
        return obj;
    }
};
