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

#include <string>
#include <exception>

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

class Value;

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

}

#include <mini_json/mini_json_generator_impl.h>

#endif /* HF5BCC136_DBD1_4CBB_BE1D_7A78F6921BEE */
