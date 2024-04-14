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
        std::string,
        JsonArray,
        JsonObject
        >;

    struct JsonArray {
        std::vector<JsonValue> elements;

        const JsonValue& operator[](size_t index) const {
            return elements[index];
        }
        JsonValue& operator[](size_t index) {
            return elements[index];
        }
    };

    struct JsonObject {
        std::vector<std::pair<std::string, JsonValue>> members;

        const JsonValue& operator[](std::string_view key) const {
            for (const auto& [k, v] : members) {
                if (k == key) {
                    return v;
                }
            }
            throw std::runtime_error("Key not found: " + std::string(key));
        }
        JsonValue& operator[](std::string_view key) {
            for (auto& [k, v] : members) {
                if (k == key) {
                    return v;
                }
            }
            throw std::runtime_error("Key not found: " + std::string(key));
        }
    };

    constexpr JsonParser() noexcept = default;
    constexpr ~JsonParser() noexcept = default;
    constexpr JsonParser(const JsonParser& other) noexcept = default;
    constexpr JsonParser& operator=(const JsonParser& other) noexcept = default;
    constexpr JsonParser(JsonParser&& other) noexcept = default;
    constexpr JsonParser& operator=(JsonParser&& other) noexcept = default;

    constexpr JsonValue parse(std::string_view json) const {
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
        return std::holds_alternative<std::string>(value);
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

    static constexpr int toInt(const JsonValue& value) {
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

    static constexpr std::string toString(const JsonValue& value) {
        if (!isString(value)) {
            throw std::runtime_error("Value is not a string");
        }
        return std::get<std::string>(value);
    }

    static constexpr const JsonArray& toArray(const JsonValue& value) {
        if (!isArray(value)) {
            throw std::runtime_error("Value is not an array");
        }
        return std::get<JsonArray>(value);
    }

    static constexpr const JsonObject& toObject(const JsonValue& value) {
        if (!isObject(value)) {
            throw std::runtime_error("Value is not an object");
        }
        return std::get<JsonObject>(value);
    }

private:
    constexpr void skipWhitespace(std::string_view json, size_t& pos) const {
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
        if (json.find(null, pos + 1) == pos + 1) {
            pos += null.size() + 1;
            return nullptr;
        }
        throw std::runtime_error("Invalid JSON: expected 'null'");
    }

    constexpr bool parseTrue(std::string_view json, size_t& pos) const {
        constexpr auto str = std::string_view("rue");
        if (json.find(str, pos + 1) == pos + 1) {
            pos += str.size() + 1;
            return true;
        }
        throw std::runtime_error("Invalid JSON: expected 'true'");
    }

    constexpr bool parseFalse(std::string_view json, size_t& pos) const {
        constexpr auto str = std::string_view("alse");
        if (json.find(str, pos + 1) == pos + 1) {
            pos += str.size() + 1;
            return false;
        }
        throw std::runtime_error("Invalid JSON: expected 'false'");
    }

    constexpr std::string parseString(std::string_view json, size_t& pos) const {
        std::string str;
        consume(json, pos); // consume opening quote

        while (peek(json, pos) != '"') {
            if (peek(json, pos) == '\\') {
                ++pos; // skip escape character
                switch (peek(json, pos)) {
                case '"':
                case '\\':
                case '/':
                    str += consume(json, pos);
                    break;
                case 'b':
                    ++pos;
                    str += '\b';
                    break;
                case 'f':
                    ++pos;
                    str += '\f';
                    break;
                case 'n':
                    ++pos;
                    str += '\n';
                    break;
                case 'r':
                    ++pos;
                    str += '\r';
                    break;
                case 't':
                    ++pos;
                    str += '\t';
                    break;
                case 'u': {
                    ++pos;
                    uint32_t codepoint = parseUnicodeEscape(json, pos);
                    // Encode the Unicode codepoint as UTF-8
                    if (codepoint <= 0x7F) {
                        str += static_cast<char>(codepoint);
                    } else if (codepoint <= 0x7FF) {
                        str += static_cast<char>(0xC0 | (codepoint >> 6));
                        str += static_cast<char>(0x80 | (codepoint & 0x3F));
                    } else if (codepoint <= 0xFFFF) {
                        str += static_cast<char>(0xE0 | (codepoint >> 12));
                        str += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        str += static_cast<char>(0x80 | (codepoint & 0x3F));
                    } else if (codepoint <= 0x10FFFF) {
                        str += static_cast<char>(0xF0 | (codepoint >> 18));
                        str += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                        str += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        str += static_cast<char>(0x80 | (codepoint & 0x3F));
                    } else {
                        throw std::runtime_error("Invalid Unicode codepoint");
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Invalid escape sequence");
                }
            } else {
                str += consume(json, pos);
            }
        }

        consume(json, pos); // consume closing quote
        return str;
    }

    constexpr uint32_t parseUnicodeEscape(std::string_view json, size_t& pos) const {
        uint32_t codepoint = 0;
        for (int i = 0; i < 4; ++i) {
            char c = peek(json, pos);
            codepoint = (codepoint << 4) | ((c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
                                                (c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
                                                (c - '0'));
            ++pos;
        }
        return codepoint;
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
                auto key = parseString(json, pos);
                skipWhitespace(json, pos);
                if (consume(json, pos) != ':')
                    throw std::runtime_error("Invalid JSON: expected ':'");
                skipWhitespace(json, pos);
                obj.members.emplace_back(std::move(key), parseValue(json, pos));
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

constexpr bool operator==(const JsonParser::JsonValue& lhs, const JsonParser::JsonValue& rhs) {
    return std::visit(
        [](const auto& l, const auto& r) {
            using T1 = std::decay_t<decltype(l)>;
            using T2 = std::decay_t<decltype(r)>;

            if constexpr (std::is_same_v<T1, std::nullptr_t> && std::is_same_v<T2, std::nullptr_t>) {
                return true;
            } else if constexpr (std::is_same_v<T1, bool> && std::is_same_v<T2, bool>) {
                return l == r;
            } else if constexpr (std::is_same_v<T1, int> && std::is_same_v<T2, int>) {
                return l == r;
            } else if constexpr (std::is_same_v<T1, double> && std::is_same_v<T2, double>) {
                return l == r;
            } else if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string>) {
                return l == r;
            }
            else if constexpr (std::is_same_v<T1, std::string_view> && std::is_same_v<T2, std::string>) {
                return l == r;
            }
            else if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string_view>) {
                return l == r;
            }
            else if constexpr (std::is_same_v<T1, JsonParser::JsonArray> && std::is_same_v<T2, JsonParser::JsonArray>) {
                return l.elements == r.elements;
            } else if constexpr (std::is_same_v<T1, JsonParser::JsonObject> && std::is_same_v<T2, JsonParser::JsonObject>) {
                return l.members == r.members;
            } else {
                return false;
            }
        },
        lhs, rhs);
}

constexpr bool operator==(const std::vector<std::pair<std::string, JsonParser::JsonValue>>& lhs,
                          const std::vector<std::pair<std::string, JsonParser::JsonValue>>& rhs) {
    if (lhs.size() != rhs.size())
        return false;

    return std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                      [](const auto& l, const auto& r) {
                          return l.first == r.first && l.second == r.second;
                      });
}
