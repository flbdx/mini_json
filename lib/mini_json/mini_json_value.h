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

#ifndef H9C59888E_77BA_4AAE_91A1_5880DF770EA7
#define H9C59888E_77BA_4AAE_91A1_5880DF770EA7

#include <cstddef>
#include <cstdint>
#include <cfloat>
#include <cmath>

#include <string>
#include <map>
#include <list>
#include <any>
#include <numeric>
#include <exception>

#include "utf_conv.h"

namespace MiniJSON {

/* forward declaration */
class Value;

/* Available Value types */
enum Type {
    Null,       /* void */
    Boolean,    /* bool */
    UInt32,     /* uint32_t */
    Int32,      /* int32_t */
    UInt64,     /* uint64_t */
    Int64,      /* int64_t */
    Float,      /* float */
    Double,     /* double */
    String,     /* std::string */
    Object,     /* std::map<std::string, Value> */
    Array       /* std::list<Value> */
};

/* Value type to C++ type */
typedef std::map<std::string, Value> ObjectValues;
typedef std::list<Value> ArrayValues;

template<Type T> struct TypeToNative {};
template<> struct TypeToNative<Type::Null>    { typedef void type; };
template<> struct TypeToNative<Type::Boolean> { typedef bool type; };
template<> struct TypeToNative<Type::UInt32>  { typedef uint32_t type; };
template<> struct TypeToNative<Type::Int32>   { typedef int32_t type; };
template<> struct TypeToNative<Type::UInt64>  { typedef uint64_t type; };
template<> struct TypeToNative<Type::Int64>   { typedef int64_t type; };
template<> struct TypeToNative<Type::Float>   { typedef float type; };
template<> struct TypeToNative<Type::Double>  { typedef double type; };
template<> struct TypeToNative<Type::String>  { typedef std::string type; };
template<> struct TypeToNative<Type::Object>  { typedef ObjectValues type; };
template<> struct TypeToNative<Type::Array>   { typedef ArrayValues type; };

/* thrown when a string value doesn't contains a valid UTF-8 sequence */
class BadEncodingException : public std::exception {
public:
    const virtual char* what() const noexcept override {
        return "UTF-8 decoding Exception";
    }
};

/* thrown when a numeric value is not allowed by the JSON standard */
class BadValueException : public std::exception {
public:
    const virtual char* what() const noexcept override {
        return "Value is not permitted by the JSON standard";
    }
};

/* A JSON value */
class Value {
public:
    Value() : m_type(Null), m_value() {}
    Value(bool v) : m_type(Boolean), m_value(v) {}
    Value(uint32_t v) : m_type(UInt32), m_value(v) {}
    Value(int32_t v) : m_type(Int32), m_value(v) {}
    Value(uint64_t v) : m_type(UInt64), m_value(v) {}
    Value(int64_t v) : m_type(Int64), m_value(v) {}
    Value(float v) :
        m_type(Float), m_value(v) {
        if (!std::isfinite(v)) {
            throw BadValueException();
        }
    }
    Value(double v) :
        m_type(Double), m_value(v) {
        if (!std::isfinite(v)) {
            throw BadValueException();
        }
    }
    Value(const std::string &v) : m_type(String), m_value(v) {}
    Value(const char *v) :  m_type(String), m_value(std::string(v)) {}
    Value(const ObjectValues &v) : m_type(Object), m_value(v) {}
    Value(const ArrayValues &v): m_type(Array), m_value(v) {}

    Value(const Value &o) : m_type(o.m_type), m_value(o.m_value) {}
    Value & operator=(const Value &o) {
        m_type = o.m_type;
        m_value = o.m_value;
        return *this;
    }
    
    /* Returns an empty object value */
    static Value new_object() {
        return Value(ObjectValues{});
    }

    /* Returns an empty array value */
    static Value new_array() {
        return Value(ArrayValues{});
    }
    
    bool operator==(const Value &o) const;

    Type get_type() const {
        return m_type;
    }

    template<Type dt>
    const typename TypeToNative<dt>::type & get() const {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<const T &>(m_value);
    }
    template<Type dt>
    typename TypeToNative<dt>::type & get() {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<T &>(m_value);
    }

    template<Type dt>
    const typename TypeToNative<dt>::type * get_ptr() const {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<const T>(&m_value);
    }
    template<Type dt>
    typename TypeToNative<dt>::type * get_ptr() {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<T>(&m_value);
    }
    
    /* produce a compact document from the value */
    std::string to_string() const;
    
    /* Produce an intended document from the value */
    std::string to_string_pretty(unsigned int indent = 4) const;

private:
    Type m_type;        /* data type of this value */
    std::any m_value;   /* Actual value, whose type is TypeToNative<m_type>::type */
    
    /* Compare 2 any values with the same data type */
    template <Type dt>
    static bool value_equal(const std::any &a, const std::any &b) {
        const auto &va = std::any_cast<const typename TypeToNative<dt>::type &>(a);
        const auto &vb = std::any_cast<const typename TypeToNative<dt>::type &>(b);
        return va == vb;
    }
    
    /* Escape a string value
     * In addition to the required escaped characters :
     * - all characters between U+00FE and U+FFFF are escaped with a \uXXXX sequence
     * - characters above 0xFFFF are escaped with a surrogate UTF-16 pair
     */
    static std::string escape_string(const std::string &in);
    
    std::string to_string_pretty_priv(const std::string &space, const std::string &add_space, const std::string &prefix) const;
};

inline bool Value::operator==(const Value &o) const {
    if (m_type != o.m_type) {
        return false;
    }
    switch (m_type) {
    case Type::Null:
        return true;
    case Type::Boolean:
        return value_equal<Type::Boolean>(m_value, o.m_value);
    case Type::UInt32:
        return value_equal<Type::UInt32>(m_value, o.m_value);
    case Type::Int32:
        return value_equal<Type::Int32>(m_value, o.m_value);
    case Type::UInt64:
        return value_equal<Type::UInt64>(m_value, o.m_value);
    case Type::Int64:
        return value_equal<Type::Int64>(m_value, o.m_value);
    case Type::Float:
        return value_equal<Type::Float>(m_value, o.m_value);
    case Type::Double:
        return value_equal<Type::Double>(m_value, o.m_value);
    case Type::String:
        return value_equal<Type::String>(m_value, o.m_value);
    case Type::Object:
        return value_equal<Type::Object>(m_value, o.m_value);
    case Type::Array:
        return value_equal<Type::Array>(m_value, o.m_value);
    }
    return false;
}

inline std::string Value::to_string_pretty_priv(const std::string &space, const std::string &add_space, const std::string &prefix) const {
    switch (m_type) {
    case Type::Null:
    case Type::Boolean:
    case Type::UInt32:
    case Type::Int32:
    case Type::UInt64:
    case Type::Int64:
    case Type::Float:
    case Type::Double:
    case Type::String:
        return space + prefix + to_string();
    case Type::Array:
    {
        const auto &list = get<Type::Array>();
        if (list.size() == 0) {
            return space + prefix + "[]";
        }
        const std::string next_space = add_space + space;

        auto tf = [&next_space, &add_space](const Value &v) {return v.to_string_pretty_priv(next_space, add_space, {});};
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ",\n" + s2;};
        auto it = list.begin();
        auto ret = std::transform_reduce(std::next(it), list.end(), tf(*it), binop, tf);
        ret = space + prefix + "[\n" + ret + "\n" + space + "]";
        return ret;
    }
    case Type::Object:
    {
        const auto &map = get<Type::Object>();
        if (map.size() == 0) {
            return space + prefix + "{}";
        }
        const std::string next_space = add_space + space;

        typedef ObjectValues::value_type value_type;
        auto tf = [&next_space, &add_space](const value_type &p) {
            return p.second.to_string_pretty_priv(next_space, add_space, escape_string(p.first) + std::string(" : "));
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

inline std::string Value::escape_string(const std::string &in) {
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

        if (cp == '\n') {
            escape_seq[1] = 'n';
            escape_len = 2;
        }
        else if (cp == '\r') {
            escape_seq[1] = 'r';
            escape_len = 2;
        }
        else if (cp == '\t') {
            escape_seq[1] = 't';
            escape_len = 2;
        }
        else if (cp == '\\') {
            escape_seq[1] = '\\';
            escape_len = 2;
        }
        else if (cp == '"') {
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

inline std::string Value::to_string() const {
    switch (m_type) {
    case Type::Null:
        return "null";
    case Type::Boolean:
        return get<Type::Boolean>() ? "true" : "false";
    case Type::UInt32:
        return std::to_string(get<Type::UInt32>());
    case Type::Int32:
        return std::to_string(get<Type::Int32>());
    case Type::UInt64:
        return std::to_string(get<Type::UInt64>());
    case Type::Int64:
        return std::to_string(get<Type::Int64>());
    case Type::Float:
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.*g", FLT_DECIMAL_DIG, get<Type::Float>());
        return buf;
    }
    case Type::Double:
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.*g", DBL_DECIMAL_DIG, get<Type::Double>());
        return buf;
    }
    case Type::String:
        return escape_string(get<Type::String>());
    case Type::Array:
    {
        const auto &list = get<Type::Array>();
        if (list.size() == 0) {
            return "[]";
        }

        auto tf = [](const Value &v) {return v.to_string();};
        auto binop = [](const std::string &s1, const std::string &s2) {return s1 + ", " + s2;};
        auto it = list.begin();
        auto ret = std::transform_reduce(std::next(it), list.end(), tf(*it), binop, tf);
        ret.insert(ret.begin(), '[');
        ret.push_back(']');
        return ret;
    }
    case Type::Object:
    {
        const auto &map = get<Type::Object>();
        if (map.size() == 0) {
            return "{}";
        }

        typedef ObjectValues::value_type value_type;
        auto tf = [](const value_type &p) {
            return escape_string(p.first) + std::string(": ") + p.second.to_string();
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

inline std::string Value::to_string_pretty(unsigned int indent) const {
    std::string add_space(indent, char(' '));
    return to_string_pretty_priv({}, add_space, {});
}

}

#endif /* H9C59888E_77BA_4AAE_91A1_5880DF770EA7 */
