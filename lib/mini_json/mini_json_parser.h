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

#ifndef H7420066C_5ED4_4AE1_AE04_49E33C75FC20
#define H7420066C_5ED4_4AE1_AE04_49E33C75FC20

#include <mini_json/mini_json_value.h>

#include <cstdlib>
#include <cerrno>
#include <string_view>

namespace MiniJSON {

/**
    * @brief Thrown when the input string is not a valid UTF-8 sequence
    * 
    */
class UTF8Exception : public std::exception {
public:
    /**
     * @brief returns an explanatory string 
     * 
     * @return message
     */
    const virtual char* what() const noexcept override {
        return "Input is not a valid UTF-8 sequence";
    }
};


/**
 * @brief Thrown when encountering a syntax error
 * 
 */
class MalFormedException : public std::exception {
    std::string m_msg;
public:
    /**
     * @brief Build a new MalFormedException
     * 
     * @param ln line number
     * @param lp line position
     * @param info message
     */
    MalFormedException(uint64_t ln, uint64_t lp, const std::string &info = {}) : m_msg() {
        m_msg = std::string("Format error line ") + std::to_string(ln) + " at position " + std::to_string(lp);
        if (!info.empty()) {
            m_msg += ": " + info;
        }
    }

    /**
     * @brief returns an explanatory string 
     * 
     * @return message
     */
    const virtual char* what() const noexcept override {
        return m_msg.c_str();
    }
};

/**
 * @brief Thrown when the recursion limit is reached
 * 
 */
class MaximumDepthException : public std::exception {
public:
    /**
     * @brief returns an explanatory string 
     * 
     * @return message
     */
    const virtual char* what() const noexcept override {
        return "Maximum recursive depth reached";
    }
};

/**
 * @brief A JSON Parser
 * 
 * This class is recursive. A maximum recursion depth can be set using setMaxDepth().
 * 
 * When parsing numbers, integer numbers will be parsed as an Int64 or UInt64 depending
 * on the sign. Unsigned integers will allways use either UInt64.
 * 
 * For floating point values, the parser always use a double representation.
 */
class Parser {
    public:
        /**
         * @brief Construct a new parser
         */
        Parser() : m_sv(), m_line_number(0), m_line_pos(0), m_depth(0), m_max_depth(1024) {
    }


    /**
     * @brief Parse a document
     * 
     * @param input document, UTF-8 encoded
     * @return JSON Value
     */
    Value parse (const std::string &input) {
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

    /**
     * @brief Get the maximum recursion depth
     * 
     * Default to 1024
     * 
     * @return uint64_t
     */
    uint64_t getMaxDepth() const {
        return m_max_depth;
    }

    /**
     * @brief Set the maximum recursion depth
     * 
     * Défault to 1024
     * 
     * @param maxDepth depth
     */
    void setMaxDepth(uint64_t maxDepth) {
        m_max_depth = maxDepth;
    }
    
private:
    std::string_view m_sv;      ///< remaining input data
    uint64_t m_line_number;     ///< current line number, from 1
    uint64_t m_line_pos;        ///< current line position, from 0
    uint64_t m_depth;           ///< current recursion depth
    uint64_t m_max_depth;       ///< configured maximum recursion depth

    /**
     * @brief Initialize the parser for a new input
     * 
     * @param input input data
     */
    void init(const std::string &input) {
        m_sv = std::string_view{input};
        m_line_number = 1;
        m_line_pos = 0;
        m_depth = 0;
    }

    /**
     * @brief Get the next unicode codepoint, without advancing the stream
     * 
     * @param cp codepoint (output)
     * @param consumed number of bytes read
     * @return false at the end of stream
     * @throws throws UTF8Exception if the stream is not valid
     */
    bool pick_codepoint(uint32_t &cp, size_t &consumed) {
        if (m_sv.size() == 0) {
            return false;
        }

        auto r = UTF::decode_one_utf8(m_sv.data(), m_sv.size(), &cp, &consumed);
        if (r != UTF::RetCode::OK) {
            throw UTF8Exception();
        }
        return true;
    }

    /**
     * @brief Advance the stream, after using pick_codepoint
     * 
     * Also update the line number and line position
     * 
     * @param cp codepoint from the last call to pick_codepoint
     * @param consumed number of bytes from the last call to pick_codepoint
     */
    void advance_codepoint(uint32_t cp, size_t consumed) {
        m_sv.remove_prefix(consumed);
        if (cp == '\n') {
            ++m_line_number;
            m_line_pos = 1;
        }
        else {
            ++m_line_pos;
        }
    }

    
    /**
     * @brief Get the next unicode codepoint
     * 
     * Equivalent to calling pick_codepoint then advance_codepoint
     * 
     * @param cp codepoint
     * @return false at the end of stream
     * @throws throws UTF8Exception if the stream is not valid
     */
    bool read_codepoint(uint32_t &cp) {
        size_t consumed;
        if (!pick_codepoint(cp, consumed)) {
            return false;
        }
        advance_codepoint(cp, consumed);
        return true;
    }

    /**
     * @brief Throws a MalFormedException
     * 
     * @param info message
     * @throws MalFormedException
     */
    [[ noreturn ]] void malformed_exception(const std::string &info = {}) {
        throw MalFormedException(m_line_number, m_line_pos, info);
    }

    /**
     * @brief Remove all the white space characters at the begining of the stream
     * 
     * @return number of chars removed
     * @throws UTF8Exception
     */
    size_t eat_ws() {
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

    /**
     * @brief Read a boolean "true" value from the stream
     * 
     * @return Value (boolean true)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_boolean_true() {
        uint32_t cp;
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
        return Value(true);
    }

    /**
     * @brief Read a boolean "false" value from the stream
     * 
     * @return Value (boolean false)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_boolean_false() {
        uint32_t cp;
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
        return Value(false);
    }

    /**
     * @brief Read a "null" value from the stream
     * 
     * @return Value (null)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_null() {
        uint32_t cp;
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
        return Value();
    }

    
    /**
     * @brief Parse an integer number
     * 
     * @param txt the text representation of the number, assumed to be valid
     * @return Value (UInt64 or Int64)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number_integer(const std::string &txt) {
        char *endptr = NULL;

        if (txt.front() == '-') {
            errno = 0;
            long long int v = strtoll(txt.c_str(), &endptr, 10);
            if (errno != 0 || endptr != txt.data() + txt.size()) {
                malformed_exception("error while parsing an integer number");
            }
            return Value(int64_t(v));
        }
        else {
            errno = 0;
            unsigned long long int v = strtoull(txt.c_str(), &endptr, 10);
            if (errno != 0 || endptr != txt.data() + txt.size()) {
                malformed_exception("error while parsing an integer number");
            }
            return Value(uint64_t(v));
        }
    }
    
    /**
     * @brief Parse an floating point number
     * 
     * @param txt the text representation of the number, assumed to be valid
     * @return Value (Double)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number_floatingpoint(const std::string &txt) {
        char *endptr = NULL;

        errno = 0;
        double d = strtod(txt.c_str(), &endptr);
        if (errno != 0 || endptr != txt.data() + txt.size()) {
            malformed_exception("error while parsing a floating-point number");
        }

        return Value(d);
    }

    /**
     * @brief Parse a JSON number
     * 
     * This method checks the syntax of the number and call either read_number_integer or read_number_floatingpoint.
     * 
     * @return Value (UInt64, Int64 or Double)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number() {
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

    /**
     * @brief Read a string from the string and unescape its content
     * 
     * @return UTF-8 string, fully decoded
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    std::string read_string_() {
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

    /**
     * @brief Read a string value from the stream
     * 
     * @return Value (String)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_string() {
        return Value(read_string_());
    }

    /**
     * @brief Read a JSON array value from the stream
     * 
     * @return Value (Array)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_array() {
        Value ret = Value::new_array();
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

    /**
     * @brief Read a JSON obejct value from the stream
     * 
     * @return Value (Object)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_object() {
        Value ret = Value::new_object();
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

    /**
     * @brief Read a JSON value from the stream
     * 
     * @return Value
     * @throws MalFormedException
     * @throws MaximumDepthException
     * @throws UTF8Exception
     */
    Value read_value() {
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
};

}

#endif /* H7420066C_5ED4_4AE1_AE04_49E33C75FC20 */
