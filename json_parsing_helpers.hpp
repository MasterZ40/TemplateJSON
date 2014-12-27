#pragma once
#ifndef __JSON_PARSING_HELPERS_HPP__
#define __JSON_PARSING_HELPERS_HPP__

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include "json_internal_declarations.hpp"

namespace tjson {
        namespace detail {

        struct Stringbuf {
            Stringbuf() {
                m_size = 1024;
                m_buf = static_cast<char*>(std::malloc(m_size));
                index = m_buf;
            }

            ~Stringbuf() {
                std::free(m_buf);
            }

            json_finline
            void push_back(char c) {
                allocate(1);
                *(index++) = c;
            }

            json_finline
            void append(const char* str, size_t len) {
                allocate(len);
                std::memcpy(index, str, len);
                index += len;
            }

            json_finline
            void append(const std::string& str) {
                append(str.c_str(), str.length());
            }

            json_finline
            std::string to_string() {
                return std::string(m_buf, std::distance(m_buf, index));
            }

        private:
            char* index;
            char* m_buf;
            size_t m_size;

            json_finline
            void allocate(size_t size) {
                size_t newsize = std::distance(m_buf, index) + size;
                if(m_size < newsize) {
                    newsize *= 1.2;
                    char* new_mem = static_cast<char*>(std::realloc(m_buf, newsize));
                    if (!new_mem) {
                        throw std::bad_alloc();
                    }
                    m_buf = new_mem;
                    m_size = newsize;
                }
            }
        };
    } /* detail */

    json_no_return inline void json_parsing_error(jsonIter iter, std::string&& errmsg) {
        jsonIter endIter = iter + std::min<size_t>(std::strlen(iter), 1000);
        std::string badJson(iter, endIter);
        throw std::invalid_argument(errmsg + ": " + badJson);
    }

    json_finline jsonIter advance_past_whitespace(jsonIter iter) {
        while(isspace(*iter)) {
            ++iter;
        }

        return iter;
    }

    json_finline jsonIter find_end_of_number(jsonIter iter) {
        if(*iter == '-') {
            ++iter;
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        if(*iter == '.') {
            ++iter;
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        if(*iter == 'e' || *iter == 'E') {
            ++iter;
            if(*iter == '-' || *iter == '+') {
                ++iter;
            }
        }

        while(isdigit(*iter)) {
            ++iter;
        }

        return iter;
    }

    json_finline jsonIter find_end_of_string(jsonIter iter) {
        bool escaping = true;

        while(iter) {
            if(*iter == '\\' || escaping) {
                escaping = !escaping;
            }
            else if(!escaping && *iter == '\"') {
                return iter;
            }

            ++iter;
        }

        return iter;
    }

    json_finline jsonIter parse_object_start(jsonIter iter) {
        iter = advance_past_whitespace(iter);
        if(*iter != '{') {
            json_parsing_error(iter, "No object start token");
        }
        ++iter;

        return iter;
    }

    json_finline jsonIter parse_object_end(jsonIter iter) {
        iter = advance_past_whitespace(iter);
        if(*iter != '}') {
            json_parsing_error(iter, "No object end token");
        }
        ++iter;
        return iter;
    }

    json_finline jsonIter parse_key_value_mapping(jsonIter iter) {
        iter = advance_past_whitespace(iter);
        if(*iter != ':') {
            json_parsing_error(iter, "Not a valid key-value mapping");
        }
        ++iter;
        return iter;
    }

    /* Advance past start quote of key string */
    json_finline jsonIter find_key_begin(jsonIter iter) {
        iter = advance_past_whitespace(iter);
        if(*iter != '\"') {
            json_parsing_error(iter, "Missing key");
        }
        return iter;
    }

    /* Advance to end quote of key string */
    json_finline jsonIter find_key_end(jsonIter iter) {
        iter = find_end_of_string(iter);
        if(*iter != '\"') {
            json_parsing_error(iter, "No close \" for key");
        }

        return iter + 1;
    }
}
#endif
