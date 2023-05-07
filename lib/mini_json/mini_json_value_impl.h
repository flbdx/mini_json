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

#ifndef H16B861EE_DE73_445A_9722_3184BA3BDA77
#define H16B861EE_DE73_445A_9722_3184BA3BDA77

#include <mini_json/mini_json_value.h>

namespace MiniJSON {

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

#endif /* H16B861EE_DE73_445A_9722_3184BA3BDA77 */
