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

#ifndef HF5BCC136_DBD1_4CBB_BE1D_7A78F6921BEE
#define HF5BCC136_DBD1_4CBB_BE1D_7A78F6921BEE

#include <mini_json/mini_json_value.h>

#include <cfloat>
#include <numeric>

namespace MiniJSON {
    
/**
 * @brief Thrown when a string value or an object key is not a valid UTF-8 string
 * 
 */
class BadEncodingException : public std::exception {
public:
    /**
     * @brief returns an explanatory string 
     * 
     * @return message
     */
    const virtual char* what() const noexcept override {
        return "UTF-8 decoding Exception";
    }
};

/**
 * @brief A JSON Document generator
 * 
 * This class is recursive so it will crash when trying to generate from a deeply recursive value.
 * 
 * The generator supports 2 forms : a compact representation or an indented representation.
 * The resulting documents are UTF-8 encoded and are actually ASCII.
 * 
 * Floating points values are encoded with enough decimals to preserve the value.S
 * 
 * This class has no state, so all methods are static.
 */
class Generator {
public:
    /**
     * @brief Produce a compact document from the value
     * 
     * @param value JSON value
     * @return JSON document
     */
    static std::string to_string(const Value &value);
    /* Produce an intended document from the value */
    /**
     * @brief Produce an indented document from the value
     * 
     * @param value JSON value
     * @param indent Number of spaces for the indentation
     * @return JSON document
     */
    static std::string to_string_pretty(const Value &value, unsigned int indent = 4);

private:
    /**
     * @brief Escape everything we need or we want to escape from a string and enclose the result between double quotes.
     * 
     * In addiction to the required escaped characters, this method also protect :
     * - all characters beween U+00FE and U+FFFF, with a single \\uXXXX sequence
     * - all characters above U+FFFF using an UTF-16 surrogate pair
     * 
     * The use of the UTF-16 surrogate pair is not mandatory in the JSON specifications,
     * so it may be incompatible with other implementations.
     * 
     * @param in string value or object key
     * @return Escaped string, ready to print
     */
    static std::string escape_string(const std::string &in);
    
    /**
     * @brief Recursive implementation of to_string_pretty
     * 
     * @param value Value to encode
     * @param space A string containing spaces for the current indentation level
     * @param add_space A string containing spaces added at each new level
     * @param prefix A prefix to output between "spacee" and the value. This argument is used to hold an object key representation
     * @return String representation of value
     */
    static std::string to_string_pretty_priv(const Value &value, const std::string &space, const std::string &add_space, const std::string &prefix);
};

inline std::string Generator::to_string_pretty_priv(const Value &value, const std::string &space, const std::string &add_space, const std::string &prefix) {
    switch (value.get_type()) {
    case Type::Null:
    case Type::Boolean:
    case Type::UInt64:
    case Type::Int64:
    case Type::Double:
    case Type::String:
        return space + prefix + to_string(value);
    case Type::Array:
    {
        const auto &list = value.get<Type::Array>();
        if (list.size() == 0) {
            return space + prefix + "[]";
        }
        const std::string next_space = add_space + space;

        auto tf = [&next_space, &add_space](const Value &v) {return to_string_pretty_priv(v, next_space, add_space, {});};
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ",\n" + s2;};
        auto it = list.begin();
        auto ret = std::transform_reduce(std::next(it), list.end(), tf(*it), binop, tf);
        ret = space + prefix + "[\n" + ret + "\n" + space + "]";
        return ret;
    }
    case Type::Object:
    {
        const auto &map = value.get<Type::Object>();
        if (map.size() == 0) {
            return space + prefix + "{}";
        }
        const std::string next_space = add_space + space;

        typedef ObjectValues::value_type value_type;
        auto tf = [&next_space, &add_space](const value_type &p) {
            return to_string_pretty_priv(p.second, next_space, add_space, escape_string(p.first) + std::string(" : "));
        };
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ",\n" + s2;};
        auto it = map.begin();
        auto ret = std::transform_reduce(std::next(it), map.end(), tf(*it), binop, tf);
        ret = space + prefix + "{\n" + ret + "\n" + space + "}";
        return ret;
    }
    }
    throw std::exception();
}

inline std::string Generator::escape_string(const std::string &in) {
    static const char *hex_encode = "0123456789ABCDEF";
    std::string out;
    out.reserve(in.size() + 2);
    out.push_back('"');

    size_t rem = in.size();
    const char *p = in.data();
    size_t consumed;
    uint32_t cp;
    char escape_seq[12] = {'\\', 0, 0, 0, 0, 0, '\\', 'u', 0, 0, 0, 0};
    int escape_len = 0;

    while (rem) {
        auto r = UTF::decode_one_utf8(p, rem, &cp, &consumed);
        if (r != UTF::RetCode::OK) {
            throw BadEncodingException();
        }

        escape_len = 0;

        if (cp == '\n') {           // mandatory (<=0x1F)
            escape_seq[1] = 'n';
            escape_len = 2;
        }
        else if (cp == '\r') {      // mandatory (<=0x1F)
            escape_seq[1] = 'r';
            escape_len = 2;
        }
        else if (cp == '\t') {      // mandatory (<=0x1F)
            escape_seq[1] = 't';
            escape_len = 2;
        }
        else if (cp == '\\') {      // mandatory
            escape_seq[1] = '\\';
            escape_len = 2;
        }
        else if (cp == '"') {       // mandatory
            escape_seq[1] = '"';
            escape_len = 2;
        }
        else if (cp < 0x20 || (cp >= 0x7F && cp <= 0xFFFF)) {
            escape_seq[1] = 'u';
            escape_seq[2] = hex_encode[(cp >> 12) & 0x0F];
            escape_seq[3] = hex_encode[(cp >>  8) & 0x0F];
            escape_seq[4] = hex_encode[(cp >>  4) & 0x0F];
            escape_seq[5] = hex_encode[(cp >>  0) & 0x0F];
            escape_len = 6;
        }
        else if (cp >= 0x10000) {
            // the rfc proposes to encode high codepoints values as an UTF-16 surrogate pair
            uint32_t hi, lo;
            cp -= 0x10000;
            hi =((cp >> 10) & 0x3FF) + 0xD800;
            lo = (cp & 0x3FF) + 0xDC00;
            escape_seq[ 1] = 'u';
            escape_seq[ 2] = hex_encode[(hi >> 12) & 0x0F];
            escape_seq[ 3] = hex_encode[(hi >>  8) & 0x0F];
            escape_seq[ 4] = hex_encode[(hi >>  4) & 0x0F];
            escape_seq[ 5] = hex_encode[(hi >>  0) & 0x0F];
            escape_seq[ 8] = hex_encode[(lo >> 12) & 0x0F];
            escape_seq[ 9] = hex_encode[(lo >>  8) & 0x0F];
            escape_seq[10] = hex_encode[(lo >>  4) & 0x0F];
            escape_seq[11] = hex_encode[(lo >>  0) & 0x0F];
            escape_len = 12;
        }

        if (escape_len > 0) {
            for (int i = 0; i < escape_len; ++i) {
                out.push_back(escape_seq[i]);
            }
        }
        else {
            UTF::encode_utf8(&cp, 1, std::back_inserter(out), NULL, NULL);
        }

        p += consumed;
        rem -= consumed;
    }
    out.push_back('"');
    return out;
}

/*This is a simpler version of escape_string_pretty_priv */
inline std::string Generator::to_string(const Value &value) {
    switch (value.get_type()) {
    case Type::Null:
        return "null";
    case Type::Boolean:
        return value.get<Type::Boolean>() ? "true" : "false";
    case Type::UInt64:
        return std::to_string(value.get<Type::UInt64>());
    case Type::Int64:
        return std::to_string(value.get<Type::Int64>());
    case Type::Double:
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.*g", DBL_DECIMAL_DIG, value.get<Type::Double>());
        return buf;
    }
    case Type::String:
        return escape_string(value.get<Type::String>());
    case Type::Array:
    {
        const auto &list = value.get<Type::Array>();
        if (list.size() == 0) {
            return "[]";
        }

        auto tf = [](const Value &v) {return to_string(v);};
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ", " + s2;};
        auto it = list.begin();
        auto ret = std::transform_reduce(std::next(it), list.end(), tf(*it), binop, tf);
        ret.insert(ret.begin(), '[');
        ret.push_back(']');
        return ret;
    }
    case Type::Object:
    {
        const auto &map = value.get<Type::Object>();
        if (map.size() == 0) {
            return "{}";
        }

        typedef ObjectValues::value_type value_type;
        auto tf = [](const value_type &p) {
            return escape_string(p.first) + std::string(": ") + to_string(p.second);
        };
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ", " + s2;};
        auto it = map.begin();
        auto ret = std::transform_reduce(std::next(it), map.end(), tf(*it), binop, tf);
        ret.insert(ret.begin(), '{');
        ret.push_back('}');
        return ret;
    }
    }
    throw std::exception();
}

inline std::string Generator::to_string_pretty(const Value &value, unsigned int indent) {
    std::string add_space(indent, char(' '));
    return to_string_pretty_priv(value, {}, add_space, {});
}

}

#endif /* HF5BCC136_DBD1_4CBB_BE1D_7A78F6921BEE */
