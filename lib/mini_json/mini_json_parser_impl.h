/*
 * Copyright 2023 Florent Bondoux
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef H6A4EA664_1696_4F20_80C0_B1B528330247
#define H6A4EA664_1696_4F20_80C0_B1B528330247

#include <mini_json/mini_json_parser.h>
#include <mini_json/mini_json_value.h>

namespace MiniJSON {
    
inline Value Parser::parse (const std::string &input) {
    init(input);

    /* eat a BOM */
    {
        uint32_t cp;
        size_t consumed;
        if (pick_codepoint(cp, consumed) && cp == 0xFEFF) {
            advance_codepoint(cp, consumed);
        }
    }

    eat_ws();
    Value v = read_value();
    eat_ws();

    // if it's not the end of the document, then is malformed (only one top level value per doc)
    if (m_sv.size() != 0) {
        malformed_exception("incorrect value (more than one top level value ?)");
    }

    return v;
}

inline bool Parser::pick_codepoint(uint32_t &cp, size_t &consumed) {
    if (m_sv.size() == 0) {
        return false;
    }

    auto r = UTF::decode_one_utf8(m_sv.data(), m_sv.size(), &cp, &consumed);
    if (r != UTF::RetCode::OK) {
        throw UTF8Exception();
    }
    return true;
}

inline void Parser::advance_codepoint(uint32_t cp, size_t consumed) {
    m_sv.remove_prefix(consumed);
    if (cp == '\n') {
        ++m_position.m_line_number;
        m_position.m_line_pos = 1;
    }
    else {
        ++m_position.m_line_pos;
    }
    ++m_position.m_offset;
}

inline bool Parser::read_codepoint(uint32_t &cp) {
    size_t consumed;
    if (!pick_codepoint(cp, consumed)) {
        return false;
    }
    advance_codepoint(cp, consumed);
    return true;
}

[[ noreturn ]] inline void Parser::malformed_exception(const std::string &info) {
    throw MalFormedException(m_position, info);
}

inline size_t Parser::eat_ws() {
    uint32_t cp;
    size_t consumed;
    size_t n_spaces = 0;
    while (true) {
        if (!pick_codepoint(cp, consumed)) {
            return n_spaces;
        }
        if (cp == ' ' || cp == '\t' || cp == '\r' || cp == '\n') {
            advance_codepoint(cp, consumed);
            ++n_spaces;
        }
        else {
            return n_spaces;
        }
    }
}

inline Value Parser::read_boolean_true() {
    uint32_t cp;
    const Position p = m_position;
    if (!read_codepoint(cp) || cp != 't') {
        malformed_exception("expected \"true\"");
    }
    if (!read_codepoint(cp) || cp != 'r') {
        malformed_exception("expected \"true\"");
    }
    if (!read_codepoint(cp) || cp != 'u') {
        malformed_exception("expected \"true\"");
    }
    if (!read_codepoint(cp) || cp != 'e') {
        malformed_exception("expected \"true\"");
    }
    return Value(true, p);
}

inline Value Parser::read_boolean_false() {
    uint32_t cp;
    const Position p = m_position;
    if (!read_codepoint(cp) || cp != 'f') {
        malformed_exception("expected \"false\"");
    }
    if (!read_codepoint(cp) || cp != 'a') {
        malformed_exception("expected \"false\"");
    }
    if (!read_codepoint(cp) || cp != 'l') {
        malformed_exception("expected \"false\"");
    }
    if (!read_codepoint(cp) || cp != 's') {
        malformed_exception("expected \"false\"");
    }
    if (!read_codepoint(cp) || cp != 'e') {
        malformed_exception("expected \"false\"");
    }
    return Value(false, p);
}

inline Value Parser::read_null() {
    uint32_t cp;
    const Position p = m_position;
    if (!read_codepoint(cp) || cp != 'n') {
        malformed_exception("expected \"null\"");
    }
    if (!read_codepoint(cp) || cp != 'u') {
        malformed_exception("expected \"null\"");
    }
    if (!read_codepoint(cp) || cp != 'l') {
        malformed_exception("expected \"null\"");
    }
    if (!read_codepoint(cp) || cp != 'l') {
        malformed_exception("expected \"null\"");
    }
    return Value(p);
}


inline Value Parser::read_number_integer(const std::string &txt) {
    char *endptr = NULL;
    const Position p = m_position;

    if (txt.front() == '-') {
        errno = 0;
        long long int v = strtoll(txt.c_str(), &endptr, 10);
        if (errno != 0 || endptr != txt.data() + txt.size()) {
            malformed_exception("error while parsing an integer number");
        }
        return Value(int64_t(v), p);
    }
    else {
        errno = 0;
        unsigned long long int v = strtoull(txt.c_str(), &endptr, 10);
        if (errno != 0 || endptr != txt.data() + txt.size()) {
            malformed_exception("error while parsing an integer number");
        }
        return Value(uint64_t(v), p);
    }
}

inline Value Parser::read_number_floatingpoint(const std::string &txt) {
    char *endptr = NULL;
    const Position p = m_position;

    errno = 0;
    double d = strtod(txt.c_str(), &endptr);
    if (errno != 0 || endptr != txt.data() + txt.size()) {
        malformed_exception("error while parsing a floating-point number");
    }

    return Value(d, p);
}

inline Value Parser::read_number() {
    std::string buf;
    uint32_t cp;
    size_t consumed;


    // minus ?
    if (!pick_codepoint(cp, consumed)) {
        malformed_exception("error while reading a number");
    }
    if (cp == '-') {
        buf.push_back(cp & 0xFF);
        advance_codepoint(cp, consumed);
    }

    // integral part
    if (!read_codepoint(cp)) {
        malformed_exception("error while reading a number (integral part)");
    }
    if (cp == '0') {
        buf.push_back(cp & 0xFF);
    }
    else if (cp >= '1' && cp <= '9') {
        buf.push_back(cp & 0xFF);
        while (true) {
            if (!pick_codepoint(cp, consumed)) {
                return read_number_integer(buf);
            }
            if (!(cp >= '0' && cp <= '9')) {
                break;
            }
            else {
                buf.push_back(cp & 0xFF);
                advance_codepoint(cp, consumed);
            }
        }

    }
    else {
        malformed_exception("error while reading a number (integral part)");
    }

    bool is_floatting_point = false;
    // frac
    if (!pick_codepoint(cp, consumed)) {
        return read_number_integer(buf);
    }
    if (cp == '.') {
        is_floatting_point = true;
        // fractional part
        buf.push_back(cp & 0xFF);
        advance_codepoint(cp, consumed);

        // at least one digit
        if (!read_codepoint(cp)) {
            malformed_exception("error while reading a number (fractional part)");
        }
        if (!(cp >= '0' && cp <= '9')) {
            malformed_exception("error while reading a number (fractional part)");
        }
        buf.push_back(cp & 0xFF);

        // other digits
        while (true) {
            if (!pick_codepoint(cp, consumed)) {
                break;
            }
            if (cp >= '0' && cp <= '9') {
                buf.push_back(cp & 0xFF);
                advance_codepoint(cp, consumed);
            }
            else {
                break;
            }
        }
    }

    // exp
    if (!pick_codepoint(cp, consumed)) {
        return is_floatting_point ? read_number_floatingpoint(buf) : read_number_integer(buf);
    }
    if (cp == 'e' || cp == 'E') {
        is_floatting_point = true;
        // exponent part
        buf.push_back(cp & 0xFF);
        advance_codepoint(cp, consumed);

        // sign or numbers
        if (!read_codepoint(cp)) {
            malformed_exception("error while reading a number (exponent part)");
        }
        if (cp == '-' || cp == '+') {
            buf.push_back(cp & 0xFF);
            if (!read_codepoint(cp)) {
                malformed_exception("error while reading a number (exponent part)");
            }
        }
        // at least one digit
        if (!(cp >= '0' && cp <= '9')) {
            malformed_exception("error while reading a number (exponent part)");
        }
        buf.push_back(cp & 0xFF);

        // other digits
        while (true) {
            if (!pick_codepoint(cp, consumed)) {
                break;
            }
            if (cp >= '0' && cp <= '9') {
                buf.push_back(cp & 0xFF);
                advance_codepoint(cp, consumed);
            }
            else {
                break;
            }
        }
    }

    return is_floatting_point ? read_number_floatingpoint(buf) : read_number_integer(buf);
}

inline std::string Parser::read_string_() {
    std::string ret;
    uint32_t cp, v;
    uint32_t x1, x2, x3, x4;

    auto is_valid_hexa = [](uint32_t v) {
        return (v >= '0' && v <= '9') || (v >= 'a' && v <= 'f') || (v >= 'A' && v <= 'F');
    };
    auto read_hexa = [](uint32_t v) -> uint32_t {
        if (v >= '0' && v <= '9') {
            return v - '0';
        }
        if (v >= 'a' && v <= 'f') {
            return v - 'a' + 10;
        }
        if (v >= 'A' && v <= 'F') {
            return v - 'A' + 10;
        }
        return 0;
    };

    // leading "
    if (!read_codepoint(cp) || cp != '"') {
        malformed_exception("error while reading a string");
    }

    while (true) {
        if (!read_codepoint(cp)) {
            malformed_exception("error while reading a string");
        }

        if (cp <= 0x1F) {
            malformed_exception("error while reading a string (invalid characters)");
        }

        if (cp == '"') {
            break;
        }

        if (cp == '\\') {
            if (!read_codepoint(cp)) {
                malformed_exception("error while reading a string (invalid escaped sequence)");
            }

            if (cp == '"' || cp == '\\' || cp == '/') {
                v = cp;
            }
            else if (cp == 'b') {
                v = 0x08;
            }
            else if (cp == 'f') {
                v = 0x0C;
            }
            else if (cp == 'n') {
                v = 0x0A;
            }
            else if (cp == 'r') {
                v = 0x0D;
            }
            else if (cp == 't') {
                v = 0x09;
            }
            else if (cp == 'u') {
                if (!read_codepoint(x1) || !read_codepoint(x2) || !read_codepoint(x3) || !read_codepoint(x4)) {
                    malformed_exception("error while reading a string (invalid escaped sequence)");
                }
                if (!is_valid_hexa(x1) || !is_valid_hexa(x2) || !is_valid_hexa(x3) || !is_valid_hexa(x4)) {
                    malformed_exception("error while reading a string (invalid escaped sequence)");
                }
                cp = (read_hexa(x1) << 12) | (read_hexa(x2) << 8) | (read_hexa(x3) << 4) | read_hexa(x4);

                if (cp >= 0xD800 && cp <= 0xDBFF) { // a high UTF-16 surrogate!
                    uint32_t hi = cp;
                    // we now expect a low surrogate...
                    if (!read_codepoint(cp) || cp != '\\') {
                        malformed_exception("error while reading a string (invalid escaped sequence)");
                    }
                    if (!read_codepoint(cp) || cp != 'u') {
                        malformed_exception("error while reading a string (invalid escaped sequence)");
                    }
                    if (!read_codepoint(x1) || !read_codepoint(x2) || !read_codepoint(x3) || !read_codepoint(x4)) {
                        malformed_exception("error while reading a string (invalid escaped sequence)");
                    }
                    if (!is_valid_hexa(x1) || !is_valid_hexa(x2) || !is_valid_hexa(x3) || !is_valid_hexa(x4)) {
                        malformed_exception("error while reading a string (invalid escaped sequence)");
                    }
                    uint32_t lo = (read_hexa(x1) << 12) | (read_hexa(x2) << 8) | (read_hexa(x3) << 4) | read_hexa(x4);
                    if (!(lo >= 0xDC00 && lo <= 0xDFFF)) {
                        malformed_exception("error while reading a string (invalid escaped sequence)");
                    }
                    v = 0x10000 + ((hi - 0xD800) << 10) + (lo - 0xDC00);
                }
                else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                    // thats a lonely low UTF-16 surrogate!
                    malformed_exception("error while reading a string (invalid escaped sequence)");
                }
                else {
                    v = cp;
                }
            }
            else {
                malformed_exception("error while reading a string (invalid escaped sequence)");
            }
            UTF::encode_utf8(&v, 1, std::back_inserter(ret), NULL, NULL);
        }
        else {
            UTF::encode_utf8(&cp, 1, std::back_inserter(ret), NULL, NULL);
        }
    }

    return ret;
}

inline Value Parser::read_string() {
    const Position p = m_position;
    return Value(read_string_(), p);
}

inline Value Parser::read_array() {
    Value ret = Value::new_array();
    ret.set_position(m_position);
    auto &array_content = ret.get<Type::Array>();

    uint32_t cp;
    size_t consumed;

    if (!read_codepoint(cp) || cp != '[') {
        malformed_exception("error while reading an array");
    }

    eat_ws();
    if (!pick_codepoint(cp, consumed)) {
        malformed_exception("error while reading an array");
    }
    if (cp == ']') {
        advance_codepoint(cp, consumed);
        return ret;
    }

    while (true) {
        eat_ws();
        array_content.push_back(read_value());
        eat_ws();

        if (!read_codepoint(cp)) {
            malformed_exception("error while reading an array");
        }
        if (cp == ']') {
            break;
        }
        if (cp != ',') {
            malformed_exception("error while reading an array");
        }
    }

    return ret;
}

inline Value Parser::read_object() {
    Value ret = Value::new_object();
    ret.set_position(m_position);
    auto &object_content = ret.get<Type::Object>();

    uint32_t cp;
    size_t consumed;
    std::string key;

    if (!read_codepoint(cp) || cp != '{') {
        malformed_exception("error while reading an object");
    }

    eat_ws();
    if (!pick_codepoint(cp, consumed)) {
        malformed_exception("error while reading an object");
    }
    if (cp == '}') {
        advance_codepoint(cp, consumed);
        return ret;
    }

    while (true) {
        eat_ws();
        key = read_string_();
        eat_ws();
        if (!read_codepoint(cp) || cp != ':') {
            malformed_exception("error while reading an object");
        }
        eat_ws();
        object_content[key] = read_value();
        eat_ws();

        if (!read_codepoint(cp)) {
            malformed_exception("error while reading an object");
        }
        if (cp == '}') {
            break;
        }
        if (cp != ',') {
            malformed_exception("error while reading an object");
        }
    }

    return ret;
}

inline Value Parser::read_value() {
    uint32_t cp;
    size_t consumed;
    if (m_depth == m_max_depth) {
        throw MaximumDepthException();
    }
    struct ScopeDepth {
        uint64_t &m_d;
        ScopeDepth(uint64_t &depth) :m_d(depth) {
            ++m_d;
        }
        ~ScopeDepth() {
            --m_d;
        }
    } depth(m_depth);

    if (!pick_codepoint(cp, consumed)) {
        malformed_exception("expected a JSON value");
    }
    if (cp == '\"') {
        return read_string();
    }
    if (cp == 'f') {
        return read_boolean_false();
    }
    if (cp == 't') {
        return read_boolean_true();
    }
    if (cp == 'n') {
        return read_null();
    }
    if (cp == '-' || (cp >= '0' && cp <= '9')) {
        return read_number();
    }
    if (cp == '{') {
        return read_object();
    }
    if (cp == '[') {
        return read_array();
    }

    malformed_exception("expected a JSON value");
}              

}

#endif /* H6A4EA664_1696_4F20_80C0_B1B528330247 */
