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
enum Type : unsigned int {
    Null = 0,       ///< 'null', underlying type is void
    Boolean = 1,    ///< 'true' or 'false', underlying type is bool
    UInt64 = 0x102, ///< number, restricted to unsigned 64 bits values
    Int64 = 0x103,  ///< number, restricted to signed 64 bits values
    Double = 0x304, ///< number, restricted to 64 bits floting point values
    String = 0x5,   ///< string, underlying type is std::string
    Object = 0x406, ///< object, underlying type is ObjectValues
    Array = 0x407   ///< array, underlying type is ArrayValues
};

/**
 * @brief A Type is a numeric type (integral or FP) if (type & MASK_TYPE_IS_NUMERIC) != 0
 * 
 */
static constexpr unsigned int MASK_TYPE_IS_NUMERIC = 0x100;
/**
 * @brief A Type is a floating point type if (type & MASK_TYPE_IS_NUMERIC_FLOAT) != 0
 * 
 */
static constexpr unsigned int MASK_TYPE_IS_NUMERIC_FLOAT = 0x200;
/**
 * @brief A Type is an array or an object if (type & MASK_TYPE_IS_CONTAINER) != 0
 * 
 */
static constexpr unsigned int MASK_TYPE_IS_CONTAINER = 0x400;

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
 * @brief Specialization of TypeToNative for UInt64
 */
template<> struct TypeToNative<Type::UInt64>  { typedef uint64_t type; /*!< unsigned 64 bits number data type */};
/**
 * @brief Specialization of TypeToNative for Int64
 */
template<> struct TypeToNative<Type::Int64>   { typedef int64_t type; /*!< signed 64 bits number data type */};
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
     * @brief Constructs a number JSON value holding unsigned 64 bits integer values
     * 
     * @param v value
     */
    Value(unsigned int v) : m_type(UInt64), m_value(uint64_t(v)) {}
    /**
     * @brief Constructs a number JSON value holding signed 64 bits integer values
     * 
     * @param v value
     */
    Value(int v) : m_type(Int64), m_value(int64_t(v)) {}
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
     * For numeric types, return true if the values are equal
     * even if the data types are different.
     * 
     * @param o other value
     * @return true if the values are identical
     */
    bool operator==(const Value &o) const;
    
    /**
     * @brief != operator
     * 
     * @param o other value
     * @return true if the values are different
     */
    bool operator!=(const Value &o) const {
        return !operator==(o);
    }

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
    bool contains(const std::string &key) const {
        return get<Type::Object>().count(key) != 0;
    }
    
    
    /**
     * @brief Assume the value is either an array, an object or a string and return its size
     * 
     * @return size of the string or number of elements of the array/object
     * @throws std::bad_any_cast if this value is neither an array, an object or a string
     */
    size_t size() const {
        switch (m_type) {
            case Type::Array:
                return get<Type::Array>().size();
            case Type::Object:
                return get<Type::Object>().size();
            case Type::String:
                return get<Type::String>().size();
            default:
                throw std::bad_any_cast();
        }
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
    
    /**
     * @brief Compare 2 numeric values (integral or floating point)
     * 
     * @param a Value of type Int64, UInt64 or Double
     * @param b Value of type Int64, UInt64 or Double
     * @return true if the two numeric values are equal
     */
    static bool numeric_equal(const Value &a, const Value &b);
};

inline bool Value::numeric_equal(const Value &a, const Value &b) {
    // compare 2 numeric values, possibly having different types
    
    const auto type_is_float = [](const Type t) { return bool(t & MASK_TYPE_IS_NUMERIC_FLOAT); };
    const auto get_sign = [](const Value &v) -> bool {
        switch (v.m_type) {
            case Type::Int64 : return v.get<Type::Int64>() < 0;
            case Type::Double : return v.get<Type::Double>() < 0;
            default:
                return false;
        }
    };
    
    // check whether they have the same sign
    bool a_is_neg = get_sign(a);
    bool b_is_neg = get_sign(b);
    if (a_is_neg != b_is_neg) {
        return false;
    }
    
    bool a_is_float = type_is_float(a.m_type);
    bool b_is_float = type_is_float(b.m_type);
    
    // both a and b have the same sign
    
    // if a and b are both either float or double
    if (a_is_float && b_is_float) {
        // cast to double and compare
        return a.get<Type::Double>() == b.get<Type::Double>();
    }
    // if a and b are (u)int(32|64), having the same sign
    else if (!(a_is_float || b_is_float)) {
        if (a_is_neg) {
            // both are negative, only possible type is Int64
            return a.get<Type::Int64>() == b.get<Type::Int64>();
        }
        else {
            // both are positive
            uint64_t va, vb;
            switch (a.m_type) {
                case Type::Int64: va = a.get<Type::Int64>(); break;
                case Type::UInt64: va = a.get<Type::UInt64>(); break;
                default: return false;
            }
            switch (b.m_type) {
                case Type::Int64: vb = b.get<Type::Int64>(); break;
                case Type::UInt64: vb = b.get<Type::UInt64>(); break;
                default: return false;
            }
            return va == vb;
        }
    }
    else {
        // one floating point value, one integer value, both having the same sign
        const auto &ra = (a_is_float) ? a : b;  // ra refers to the floating point value
        const auto &rb = (a_is_float) ? b : a;  // rb refers to the integer value
        
        double va = ra.get<Type::Double>();
        
        // ra needs to be an integral
        bool ra_is_integral = (trunc(va) == va);
        
        if (!ra_is_integral) {
            return false;
        }
        
        // if ra is integral and positive
        if (va >= 0.0) {
            // cast to integer and check that we are not out of bounds
            uint64_t ua = (uint64_t) trunc(va);
            if (va != (double) ua) {
                return false;
            }
            uint64_t vb;
            switch (rb.m_type) {
                case Type::Int64: vb = rb.get<Type::Int64>(); break;
                case Type::UInt64: vb = rb.get<Type::UInt64>(); break;
                default: return false;
            }
            return va == vb;
        }
        else {
            // ra is integral and negative
            // cast to integer and check that we are not out of bounds
            int64_t ia = (int64_t) trunc(va);
            if (va != (double) ia) {
                return false;
            }
            int64_t vb = rb.get<Type::Int64>();
            return va == vb;
        }
    }
    
    return false;
}

inline bool Value::operator==(const Value &o) const {
    const auto type_is_numeric = [](const Type t) { return bool(t & MASK_TYPE_IS_NUMERIC); };
    if (m_type != o.m_type) {
        // if the types are both numerics but different, do a "deep" comparison
        if (type_is_numeric(m_type) && type_is_numeric(o.m_type)) {
            return numeric_equal(*this, o);
        }
        return false;
    }
    switch (m_type) {
    case Type::Null:
        return true;
    case Type::Boolean:
        return value_equal<Type::Boolean>(m_value, o.m_value);
    case Type::UInt64:
        return value_equal<Type::UInt64>(m_value, o.m_value);
    case Type::Int64:
        return value_equal<Type::Int64>(m_value, o.m_value);
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
