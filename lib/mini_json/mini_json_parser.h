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

#include <cstdlib>
#include <cerrno>
#include <string_view>
#include <exception>

#include "mini_json_value.h"

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
     * @param p position
     * @param info message
     */
    MalFormedException(Position p, const std::string &info = {}) : m_msg() {
        m_msg = std::string("Format error line ") + std::to_string(p.m_line_number) + " at position " + std::to_string(p.m_line_pos) + ", offset " + std::to_string(p.m_offset);
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
        Parser() : m_sv(), m_position(), m_depth(0), m_max_depth(1024) {
    }


    /**
     * @brief Parse a document
     * 
     * @param input document, UTF-8 encoded
     * @return JSON Value
     */
    Value parse (const std::string &input);

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
    Position m_position;        ///< current position in the stream
    uint64_t m_depth;           ///< current recursion depth
    uint64_t m_max_depth;       ///< configured maximum recursion depth

    /**
     * @brief Initialize the parser for a new input
     * 
     * @param input input data
     */
    void init(const std::string &input) {
        m_sv = std::string_view{input};
        m_position = {1, 1, 0};
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
    bool pick_codepoint(uint32_t &cp, size_t &consumed);

    /**
     * @brief Advance the stream, after using pick_codepoint
     * 
     * Also update the line number and line position
     * 
     * @param cp codepoint from the last call to pick_codepoint
     * @param consumed number of bytes from the last call to pick_codepoint
     */
    void advance_codepoint(uint32_t cp, size_t consumed);

    
    /**
     * @brief Get the next unicode codepoint
     * 
     * Equivalent to calling pick_codepoint then advance_codepoint
     * 
     * @param cp codepoint
     * @return false at the end of stream
     * @throws throws UTF8Exception if the stream is not valid
     */
    bool read_codepoint(uint32_t &cp);

    /**
     * @brief Throws a MalFormedException
     * 
     * @param info message
     * @throws MalFormedException
     */
    [[ noreturn ]] void malformed_exception(const std::string &info = {});

    /**
     * @brief Remove all the white space characters at the begining of the stream
     * 
     * @return number of chars removed
     * @throws UTF8Exception
     */
    size_t eat_ws();

    /**
     * @brief Read a boolean "true" value from the stream
     * 
     * @return Value (boolean true)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_boolean_true();

    /**
     * @brief Read a boolean "false" value from the stream
     * 
     * @return Value (boolean false)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_boolean_false();

    /**
     * @brief Read a "null" value from the stream
     * 
     * @return Value (null)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_null();

    
    /**
     * @brief Parse an integer number
     * 
     * @param txt the text representation of the number, assumed to be valid
     * @return Value (UInt64 or Int64)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number_integer(const std::string &txt);
    
    /**
     * @brief Parse an floating point number
     * 
     * @param txt the text representation of the number, assumed to be valid
     * @return Value (Double)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number_floatingpoint(const std::string &txt);

    /**
     * @brief Parse a JSON number
     * 
     * This method checks the syntax of the number and call either read_number_integer or read_number_floatingpoint.
     * 
     * @return Value (UInt64, Int64 or Double)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_number();

    /**
     * @brief Read a string from the string and unescape its content
     * 
     * @return UTF-8 string, fully decoded
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    std::string read_string_();

    /**
     * @brief Read a string value from the stream
     * 
     * @return Value (String)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_string();

    /**
     * @brief Read a JSON array value from the stream
     * 
     * @return Value (Array)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_array();

    /**
     * @brief Read a JSON obejct value from the stream
     * 
     * @return Value (Object)
     * @throws MalFormedException
     * @throws UTF8Exception
     */
    Value read_object();

    /**
     * @brief Read a JSON value from the stream
     * 
     * @return Value
     * @throws MalFormedException
     * @throws MaximumDepthException
     * @throws UTF8Exception
     */
    Value read_value();
};

}

#include <mini_json/mini_json_parser_impl.h>

#endif /* H7420066C_5ED4_4AE1_AE04_49E33C75FC20 */
