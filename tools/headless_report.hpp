#pragma once
#include <cctype>
#include <map>
#include <stdexcept>
#include <string>

namespace th08::headless_report {
// Validate the entire small JSON document emitted by our child, including nested
// collision fields and duplicate keys. Retain scalar spelling so 64-bit digests
// never pass through floating point. This is a process protocol, not permissive
// extraction from arbitrary text or a replacement for the native replay oracle.
class Document {
    const std::string &text;
    std::size_t cursor = 0;
    void space() {
        while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t' ||
                                        text[cursor] == '\r' || text[cursor] == '\n'))
            ++cursor;
    }
    char peek() {
        space();
        if (cursor == text.size())
            throw std::runtime_error("truncated child JSON");
        return text[cursor];
    }
    void take(char expected) {
        if (peek() != expected)
            throw std::runtime_error("invalid child JSON punctuation");
        ++cursor;
    }
    std::string string() {
        take('"');
        const auto begin = cursor;
        while (cursor < text.size()) {
            const auto c = static_cast<unsigned char>(text[cursor++]);
            if (c == '"')
                return text.substr(begin, cursor - begin - 1);
            if (c < 32)
                throw std::runtime_error("control byte in child JSON string");
            if (c == '\\') {
                if (cursor == text.size())
                    throw std::runtime_error("truncated JSON escape");
                const char escape = text[cursor++];
                if (escape == 'u') {
                    for (int i = 0; i < 4; ++i)
                        if (cursor == text.size() ||
                            !std::isxdigit(static_cast<unsigned char>(text[cursor++])))
                            throw std::runtime_error("invalid Unicode escape");
                } else if (std::string("\"\\/bfnrt").find(escape) == std::string::npos)
                    throw std::runtime_error("invalid JSON escape");
            }
        }
        throw std::runtime_error("unterminated child JSON string");
    }
    void value(unsigned depth) {
        if (depth > 16)
            throw std::runtime_error("child JSON nesting limit");
        char c = peek();
        if (c == '{') {
            object(depth + 1);
            return;
        }
        if (c == '[') {
            ++cursor;
            if (peek() == ']') {
                ++cursor;
                return;
            }
            for (;;) {
                value(depth + 1);
                c = peek();
                ++cursor;
                if (c == ']')
                    return;
                if (c != ',')
                    throw std::runtime_error("invalid JSON array");
            }
        }
        if (c == '"') {
            string();
            return;
        }
        for (const char *literal : {"true", "false", "null"}) {
            const std::string word = literal;
            if (text.compare(cursor, word.size(), word) == 0) {
                cursor += word.size();
                return;
            }
        }
        if (c == '-') {
            ++cursor;
            if (cursor == text.size())
                throw std::runtime_error("truncated JSON number");
            c = text[cursor];
        }
        if (c == '0')
            ++cursor;
        else if (c >= '1' && c <= '9') {
            do {
                ++cursor;
            } while (cursor < text.size() &&
                     std::isdigit(static_cast<unsigned char>(text[cursor])));
        } else
            throw std::runtime_error("invalid JSON value");
        if (cursor < text.size() && text[cursor] == '.') {
            ++cursor;
            digits();
        }
        if (cursor < text.size() && (text[cursor] == 'e' || text[cursor] == 'E')) {
            ++cursor;
            if (cursor < text.size() && (text[cursor] == '+' || text[cursor] == '-'))
                ++cursor;
            digits();
        }
    }
    void digits() {
        const auto begin = cursor;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor])))
            ++cursor;
        if (cursor == begin)
            throw std::runtime_error("invalid JSON number");
    }
    std::map<std::string, std::string> object(unsigned depth) {
        take('{');
        std::map<std::string, std::string> result;
        if (peek() == '}') {
            ++cursor;
            return result;
        }
        for (;;) {
            const auto key = string();
            if (key.find('\\') != std::string::npos)
                throw std::runtime_error("escaped protocol key");
            take(':');
            space();
            const auto begin = cursor;
            value(depth);
            if (!result.emplace(key, text.substr(begin, cursor - begin)).second)
                throw std::runtime_error("duplicate child JSON key");
            const char c = peek();
            ++cursor;
            if (c == '}')
                return result;
            if (c != ',')
                throw std::runtime_error("invalid JSON object");
        }
    }

  public:
    explicit Document(const std::string &input) : text(input) {}
    std::map<std::string, std::string> parse() {
        auto result = object(0);
        space();
        if (cursor != text.size())
            throw std::runtime_error("trailing child JSON content");
        return result;
    }
};
inline std::string field(const std::string &report, const std::string &key) {
    const auto fields = Document(report).parse();
    auto found = fields.find(key);
    if (found == fields.end())
        throw std::runtime_error("missing child field: " + key);
    return found->second;
}
} // namespace th08::headless_report
