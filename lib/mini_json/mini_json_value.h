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
#include <cmath>

#include <string>
#include <map>
#include <list>
#include <any>
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

}

#endif /* H9C59888E_77BA_4AAE_91A1_5880DF770EA7 */
