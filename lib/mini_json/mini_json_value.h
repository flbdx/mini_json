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

class Value;

/**
 * @brief Type of the value stored in a Value object
 * 
 */
enum Type {
    Null,       ///< 'null', underlying type is void
    Boolean,    ///< 'true' or 'false', underlying type is bool
    UInt32,     ///< number, restricted to unsigned 32 bits values
    Int32,      ///< number, restricted to signed 32 bits values
    UInt64,     ///< number, restricted to unsigned 64 bits values
    Int64,      ///< number, restricted to signed 64 bits values
    Float,      ///< number, restricted to 32 bits floting point values
    Double,     ///< number, restricted to 64 bits floting point values
    String,     ///< string, underlying type is std::string
    Object,     ///< object, underlying type is ObjectValues
    Array       ///< array, underlying type is ArrayValues
};

/**
 * @brief The underlying type of a Value representing a JSON object
 * 
 */
typedef std::map<std::string, Value> ObjectValues;
/**
 * @brief The underlying type of a Value representing a JSON array
 * 
 */
typedef std::list<Value> ArrayValues;

/**
 * @brief A templated struct holding the mapping between the Type enum and the actual data type
 * 
 * The TypeToNative struct defines a 'type' typename holding the underlying type
 */
template<Type T> struct TypeToNative {};
/**
 * @brief Specialization of TypeToNative for Null
 */
template<> struct TypeToNative<Type::Null>    { typedef void type; /*!< Null data type */};
/**
 * @brief Specialization of TypeToNative for Boolean
 */
template<> struct TypeToNative<Type::Boolean> { typedef bool type; /*!< Boolean data type */};
/**
 * @brief Specialization of TypeToNative for UInt32
 */
template<> struct TypeToNative<Type::UInt32>  { typedef uint32_t type; /*!< unsigned 32 bits number data type */};
/**
 * @brief Specialization of TypeToNative for Int32
 */
template<> struct TypeToNative<Type::Int32>   { typedef int32_t type; /*!< signed 32 bits number data type */};
/**
 * @brief Specialization of TypeToNative for UInt64
 */
template<> struct TypeToNative<Type::UInt64>  { typedef uint64_t type; /*!< unsigned 64 bits number data type */};
/**
 * @brief Specialization of TypeToNative for Int64
 */
template<> struct TypeToNative<Type::Int64>   { typedef int64_t type; /*!< signed 64 bits number data type */};
/**
 * @brief Specialization of TypeToNative for Float
 */
template<> struct TypeToNative<Type::Float>   { typedef float type; /*!< 32 bits floating point number data type */};
/**
 * @brief Specialization of TypeToNative for Double
 */
template<> struct TypeToNative<Type::Double>  { typedef double type; /*!< 64 bits floating point number data type */};
/**
 * @brief Specialization of TypeToNative for String
 */
template<> struct TypeToNative<Type::String>  { typedef std::string type; /*!< string data type */};
/**
 * @brief Specialization of TypeToNative for Object
 */
template<> struct TypeToNative<Type::Object>  { typedef ObjectValues type; /*!< JSON object data type */};
/**
 * @brief Specialization of TypeToNative for Array
 */
template<> struct TypeToNative<Type::Array>   { typedef ArrayValues type; /*!< JSON string data type */};

/**
 * @brief Thrown when a numeric value is not allowed by the JSON standard
 * 
 * When trying t create a value from a infinite or NaN floating point number.
 */
class BadValueException : public std::exception {
public:
    /**
     * @brief returns an explanatory string 
     * 
     * @return message
     */
    const virtual char* what() const noexcept override {
        return "Value is not permitted by the JSON standard";
    }
};

/**
 * @brief A JSON Value
 * 
 * The Value objects have two member variables : m_type, holding its data type, and m_value, a std::any holding the content.
 * Depending of the size of the content, there may be dynamic memory allocation to store the content.
 */
class Value {
public:
    /**
     * @brief Constructs a null JSON value
     * 
     */
    Value() : m_type(Null), m_value() {}
    /**
     * @brief Constructs a boolean JSON value
     * 
     * @param v value
     */
    Value(bool v) : m_type(Boolean), m_value(v) {}
    /**
     * @brief Constructs a number JSON value holding unsigned 32 bits integer values
     * 
     * @param v value
     */
    Value(uint32_t v) : m_type(UInt32), m_value(v) {}
    /**
     * @brief Constructs a number JSON value holding signed 32 bits integer values
     * 
     * @param v value
     */
    Value(int32_t v) : m_type(Int32), m_value(v) {}
    /**
     * @brief Constructs a number JSON value holding unsigned 64 bits integer values
     * 
     * @param v value
     */
    Value(uint64_t v) : m_type(UInt64), m_value(v) {}
    /**
     * @brief Constructs a number JSON value holding signed 64 bits integer values
     * 
     * @param v value
     */
    Value(int64_t v) : m_type(Int64), m_value(v) {}
    /**
     * @brief Constructs a number JSON value holding 32 bits floating point values
     * 
     * @param v value, must not be infinity or NaN
     */
    Value(float v) :
        m_type(Float), m_value(v) {
        if (!std::isfinite(v)) {
            throw BadValueException();
        }
    }
    /**
     * @brief Constructs a number JSON value holding 64 bits floating point values
     * 
     * @param v value, must not be infinity or NaN
     */
    Value(double v) :
        m_type(Double), m_value(v) {
        if (!std::isfinite(v)) {
            throw BadValueException();
        }
    }
    /**
     * @brief Constructs a string JSON value
     * 
     * @param v value, must be UTF-8 encoded
     */
    Value(const std::string &v) : m_type(String), m_value(v) {}
    /**
     * @brief Constructs a string JSON value
     * 
     * @param v value, must be UTF-8 encoded
     */
    Value(const char *v) :  m_type(String), m_value(std::string(v)) {}
    /**
     * @brief Constructs an object JSON value
     * 
     * @param v value, the keys must be UTF-8 encoded
     */
    Value(const ObjectValues &v) : m_type(Object), m_value(v) {}
    /**
     * @brief Constructs an array JSON value
     * 
     * @param v value
     */
    Value(const ArrayValues &v): m_type(Array), m_value(v) {}

    /**
     * @brief Copy constructor
     * 
     * @param o JSON value
     */
    Value(const Value &o) : m_type(o.m_type), m_value(o.m_value) {}
    
    
    /**
     * @brief Assignment operator
     * 
     * @param o JSON value
     * @return reference to this
     */
    Value & operator=(const Value &o) {
        m_type = o.m_type;
        m_value = o.m_value;
        return *this;
    }
    
    /**
     * @brief Returns an empty JSON object value
     * 
     * @return JSON value
     */
    static Value new_object() {
        return Value(ObjectValues{});
    }

    /**
     * @brief Returns an empty JSON array value
     * 
     * @return JSON value
     */    static Value new_array() {
        return Value(ArrayValues{});
    }
    
    /**
     * @brief equal operator
     * 
     * @param o othjer value
     * @return true if the data types and values are identical
     */
    bool operator==(const Value &o) const;

    /**
     * @brief Returns the type of the value
     * 
     * @return type
     */
    Type get_type() const {
        return m_type;
    }

    
    /**
     * @brief Get a const reference on the object content
     * 
     * The template argument must be equal to the type returned by get_type(). 
     * 
     * @tparam dt Must be equal to the data type of the object
     * @return the value's content
     * @throws throws std::bad_any_cast if the template argument does not match the actual data type
     */
    template<Type dt> const typename TypeToNative<dt>::type & get() const {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<const T &>(m_value);
    }
    /**
     * @brief Get a reference on the object content
     * 
     * The template argument must be equal to the type returned by get_type(). 
     * 
     * @tparam dt Must be equal to the data type of the object
     * @return the value's content
     * @throws throws std::bad_any_cast if the template argument does not match the actual data type
     */
    template<Type dt> typename TypeToNative<dt>::type & get() {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<T &>(m_value);
    }

    /**
     * @brief Get a const pointer to the object content
     * 
     * The template argument must be equal to the type returned by get_type().
     * Unlike the get() methods, get_ptr() does not throw an exception if the type is invalid.
     * 
     * @tparam dt Must be equal to the data type of the object
     * @return a pointer to the value's content
     */
    template<Type dt> const typename TypeToNative<dt>::type * get_ptr() const {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<const T>(&m_value);
    }
    /**
     * @brief Get a pointer to the object content
     * 
     * The template argument must be equal to the type returned by get_type().
     * Unlike the get() methods, get_ptr() does not throw an exception if the type is invalid.
     * 
     * @tparam dt Must be equal to the data type of the object
     * @return a pointer to the value's content
     */
    template<Type dt> typename TypeToNative<dt>::type * get_ptr() {
        typedef typename TypeToNative<dt>::type T;
        return std::any_cast<T>(&m_value);
    }
    
    /**
     * @brief Assume the value is of type Object and access a value by its key.
     * 
     * @param key The key to retrieve
     * @return Value
     * @throws std::out_of_range if the key is not defined
     * @throws std::bad_any_cast if this value is not an Object
     */
    const Value & operator[](const std::string &key) const {
        return get<Type::Object>().at(key);
    }
    
    /**
     * @brief Assume the value is of type Object and access or insert a value by its key.
     * 
     * @param key The key to retrieve
     * @return Value
     * @throws std::out_of_range if the key is not defined
     * @throws std::bad_any_cast if this value is not an Object
     */
    Value & operator[](const std::string &key) {
        return get<Type::Object>()[key];
    }
    
    /**
     * @brief Assume the value is of type Object and test whether a key is defined
     * 
     * @param key The key to test
     * @return true if the key is defined
     * @throws std::bad_any_cast if this value is not an Object
     */
    bool keyExists(const std::string &key) const {
        return get<Type::Object>().count(key) != 0;
    }

private:
    Type m_type;        ///< data type of this value
    std::any m_value;   ///< Actual value, whose type is TypeToNative<m_type>::type
    
    /* Compare 2 any values with the same data type */
    /**
     * @brief Compare two std::any assuming they are holding the same data type and the operator == is defined
     * 
     * @tparam dt Data type
     * @param a std::any holding a TypeToNative<dt>::type value
     * @param b std::any holding a TypeToNative<dt>::type value
     * @return true if the values are equal
     */
    template <Type dt> static bool value_equal(const std::any &a, const std::any &b) {
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
