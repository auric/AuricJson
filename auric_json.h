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

struct JsonValue {
public:
    struct Array;
    struct Object;

    using ValueType = std::variant<
        std::nullptr_t,
        bool,
        int,
        double,
        std::string,
        Array,
        Object
        >;

    struct Array {
        std::vector<JsonValue> elements;

        const JsonValue& operator[](size_t index) const {
            return elements[index];
        }
        JsonValue& operator[](size_t index) {
            return elements[index];
        }
    };

    struct Object {
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

    constexpr JsonValue() = default;
    constexpr JsonValue(const ValueType& value) : value(value) {}
    constexpr JsonValue(ValueType&& value) : value(std::move(value)) {}

    constexpr JsonValue(std::nullptr_t) : value(nullptr) {}
    constexpr JsonValue(bool val) : value(val) {}
    constexpr JsonValue(int val) : value(val) {}
    constexpr JsonValue(double val) : value(val) {}
    constexpr JsonValue(const std::string& val) : value(val) {}
    constexpr JsonValue(std::string&& val) : value(std::move(val)) {}
    constexpr JsonValue(std::string_view val) : value(std::string(val)) {}
    constexpr JsonValue(const char* val) : value(std::string(val)) {}
    constexpr JsonValue(const Array& arr) : value(arr) {}
    constexpr JsonValue(Array&& arr) : value(std::move(arr)) {}
    constexpr JsonValue(const Object& obj) : value(obj) {}
    constexpr JsonValue(Object&& obj) : value(std::move(obj)) {}

    constexpr bool isNull() const {
        return JsonValue::isNull(*this);
    }

    constexpr bool isBool() const {
        return JsonValue::isBool(*this);
    }

    constexpr bool isInt() const {
        return JsonValue::isInt(*this);
    }

    constexpr bool isDouble() const {
        return JsonValue::isDouble(*this);
    }

    constexpr bool isString() const {
        return JsonValue::isString(*this);
    }

    constexpr bool isArray() const {
        return JsonValue::isArray(*this);
    }

    constexpr bool isObject() const {
        return JsonValue::isObject(*this);
    }

    constexpr bool toBool() const {
        return JsonValue::toBool(*this);
    }

    constexpr int toInt() const {
        return JsonValue::toInt(*this);
    }

    constexpr double toDouble() const {
        return JsonValue::toDouble(*this);
    }

    constexpr std::string toString() const {
        return JsonValue::toString(*this);
    }

    constexpr Array toArray() const {
        return JsonValue::toArray(*this);
    }

    constexpr Object toObject() const {
        return JsonValue::toObject(*this);
    }

    static constexpr bool isNull(const JsonValue& value) {
        return std::holds_alternative<std::nullptr_t>(value.value);
    }

    static constexpr bool isBool(const JsonValue& value) {
        return std::holds_alternative<bool>(value.value);
    }

    static constexpr bool isInt(const JsonValue& value) {
        return std::holds_alternative<int>(value.value);
    }

    static constexpr bool isDouble(const JsonValue& value) {
        return std::holds_alternative<double>(value.value);
    }

    static constexpr bool isString(const JsonValue& value) {
        return std::holds_alternative<std::string>(value.value);
    }

    static constexpr bool isArray(const JsonValue& value) {
        return std::holds_alternative<Array>(value.value);
    }

    static constexpr bool isObject(const JsonValue& value) {
        return std::holds_alternative<Object>(value.value);
    }

    static constexpr bool toBool(const JsonValue& value) {
        if (!isBool(value)) {
            throw std::runtime_error("Value is not a boolean");
        }
        return std::get<bool>(value.value);
    }

    static constexpr int toInt(const JsonValue& value) {
        if (!isInt(value)) {
            throw std::runtime_error("Value is not an integer");
        }
        return std::get<int>(value.value);
    }

    static constexpr double toDouble(const JsonValue& value) {
        if (!isDouble(value)) {
            throw std::runtime_error("Value is not a double");
        }
        return std::get<double>(value.value);
    }

    static constexpr std::string toString(const JsonValue& value) {
        if (!isString(value)) {
            throw std::runtime_error("Value is not a string");
        }
        return std::get<std::string>(value.value);
    }

    static constexpr Array toArray(const JsonValue& value) {
        if (!isArray(value)) {
            throw std::runtime_error("Value is not an array");
        }
        return std::get<Array>(value.value);
    }

    static constexpr Object toObject(const JsonValue& value) {
        if (!isObject(value)) {
            throw std::runtime_error("Value is not an object");
        }
        return std::get<Object>(value.value);
    }

    ValueType value;
};

constexpr bool operator==(const JsonValue& lhs, const JsonValue& rhs) {
    return lhs.value == rhs.value;
}

class JsonParser {
public:
    constexpr JsonParser() noexcept = default;
    constexpr ~JsonParser() noexcept = default;
    constexpr JsonParser(const JsonParser& other) noexcept = default;
    constexpr JsonParser& operator=(const JsonParser& other) noexcept = default;
    constexpr JsonParser(JsonParser&& other) noexcept = default;
    constexpr JsonParser& operator=(JsonParser&& other) noexcept = default;

    static constexpr JsonValue parse(std::string_view json) {
        size_t pos = 0;
        skipWhitespace(json, pos);
        return parseValue(json, pos);
    }

private:
    static constexpr void skipWhitespace(std::string_view json, size_t& pos) {
        while (pos < json.size() && isspace(json[pos]))
            ++pos;
    }

    static constexpr char peek(std::string_view json, size_t pos) {
        if (pos >= json.size())
            throw std::runtime_error("Unexpected end of JSON");
        return json[pos];
    }

    static constexpr char consume(std::string_view json, size_t& pos) {
        if (pos >= json.size())
            throw std::runtime_error("Unexpected end of JSON");
        return json[pos++];
    }

    static constexpr JsonValue parseValue(std::string_view json, size_t& pos) {
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

    static constexpr std::nullptr_t parseNull(std::string_view json, size_t& pos) {
        constexpr auto null = std::string_view("ull");
        if (json.find(null, pos + 1) == pos + 1) {
            pos += null.size() + 1;
            return nullptr;
        }
        throw std::runtime_error("Invalid JSON: expected 'null'");
    }

    static constexpr bool parseTrue(std::string_view json, size_t& pos) {
        constexpr auto str = std::string_view("rue");
        if (json.find(str, pos + 1) == pos + 1) {
            pos += str.size() + 1;
            return true;
        }
        throw std::runtime_error("Invalid JSON: expected 'true'");
    }

    static constexpr bool parseFalse(std::string_view json, size_t& pos) {
        constexpr auto str = std::string_view("alse");
        if (json.find(str, pos + 1) == pos + 1) {
            pos += str.size() + 1;
            return false;
        }
        throw std::runtime_error("Invalid JSON: expected 'false'");
    }

    static constexpr std::string parseString(std::string_view json, size_t& pos) {
        std::string str;
        consume(json, pos); // consume opening quote
        while (true) {
            char c = peek(json, pos);
            if (c == '"') {
                ++pos; // consume closing quote
                break;
            } else if (c == '\\') {
                ++pos; // skip escape character
                c = peek(json, pos);
                switch (c) {
                case '"':
                case '\\':
                case '/':
                    str.push_back(c);
                    ++pos;
                    break;
                case 'b': str.push_back('\b'); ++pos; break;
                case 'f': str.push_back('\f'); ++pos; break;
                case 'n': str.push_back('\n'); ++pos; break;
                case 'r': str.push_back('\r'); ++pos; break;
                case 't': str.push_back('\t'); ++pos; break;
                case 'u': {
                    ++pos;
                    uint32_t codepoint = parseUnicodeEscape(json, pos);
                    encodeUTF8(str, codepoint);
                    break;
                }
                default:
                    throw std::runtime_error("Invalid escape sequence");
                }
            } else {
                str.push_back(c);
                ++pos;
            }
        }
        return str;
    }

    static constexpr uint32_t parseUnicodeEscape(std::string_view json, size_t& pos) {
        uint32_t codepoint = 0;
        for (int i = 0; i < 4; ++i) {
            char c = peek(json, pos);
            uint8_t digit = (c >= 'A') ? (c & 0xDF) - 'A' + 10 : c - '0'; // Directly compute hexadecimal value
            codepoint = (codepoint << 4) | digit;
            ++pos;
        }
        return codepoint;
    }

    static constexpr void encodeUTF8(std::string& str, uint32_t codepoint) {
        if (codepoint <= 0x7F) {
            str.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            str.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0xFFFF) {
            str.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else if (codepoint <= 0x10FFFF) {
            str.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            str.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            str.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            str.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else {
            throw std::runtime_error("Invalid Unicode codepoint");
        }
    }

    static JsonValue parseNumber(std::string_view json, size_t& pos) {
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

    static JsonValue parseArray(std::string_view json, size_t& pos) {
        JsonValue::Array arr;
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
                if (peek(json, pos) == ']')
                    break; // Allow trailing comma
            }
            arr.elements.reserve(elemCount); // Avoid reallocations
        }
        consume(json, pos); // consume closing bracket
        return arr;
    }

    static JsonValue parseObject(std::string_view json, size_t& pos) {
        JsonValue::Object obj;
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
                if (peek(json, pos) == '}')
                    break; // Allow trailing comma
            }
            obj.members.reserve(memberCount); // Avoid reallocations
        }
        consume(json, pos); // consume closing brace
        return obj;
    }
};

