#pragma once
#ifndef __JSON_TUPLE_PARSER_HPP__
#define __JSON_TUPLE_PARSER_HPP__

/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

#include "json_internal_declarations.hpp"

namespace tjson {
    namespace detail {
        template<typename TupleType,
                 size_t curIndex,
                 bool lastValue,
                 typename curType,
                 typename... Types>
        struct TupleHandler : reference_only {
            json_finline static void to_json(const TupleType& classFrom, Stringbuf& out) {
                detail::to_json(std::get<curIndex>(classFrom), out);
                out.push_back(',');
                TupleHandler<TupleType,
                             curIndex + 1,
                             sizeof...(Types) == 1,
                             Types...
                            >::to_json(classFrom, out);
            }

            json_finline static jsonIter from_json(jsonIter iter, DataMember<TupleType>& into) {
                DataMember<curType> data;

                iter = detail::from_json(iter, data);
                std::get<curIndex>(into.access()) = data.consume();

                iter = advance_past_whitespace(iter);
                if (*iter != ',') {
                    json_parsing_error(iter, "Not a valid tuple value");
                }
                ++iter;
                return TupleHandler<TupleType,
                                    curIndex + 1,
                                    sizeof...(Types) == 1,
                                    Types...
                                   >::from_json(iter, into);
            }
        };

        template<typename TupleType,
                 size_t curIndex,
                 typename curType,
                 typename... Types>
        struct TupleHandler<TupleType, curIndex, true, curType, Types...> : reference_only {

            json_finline static void to_json(const TupleType& classFrom, Stringbuf& out) {
                detail::to_json(std::get<curIndex>(classFrom), out);
            }

            json_finline static jsonIter from_json(jsonIter iter, DataMember<TupleType>& into) {
                DataMember<curType> data;

                iter = detail::from_json(iter, data);
                std::get<curIndex>(into.access()) = data.consume();

                iter = advance_past_whitespace(iter);
                if (* iter != ']') {
                    json_parsing_error(iter, "No tuple end token");
                }
                ++iter;
                return iter;
            }
        };
    } /* detail */

    template<typename... Types>
    void to_json(const std::tuple<Types...>& from, detail::Stringbuf& out) {
        out.push_back('[');
        detail::TupleHandler<std::tuple<Types...>,
                             0,
                             sizeof...(Types) == 1,
                             Types...
                            >::to_json(from, out);
        out.push_back(']');
    }

    template<typename... Types>
    jsonIter from_json(jsonIter iter, detail::DataMember<std::tuple<Types...>>& into) {
        if(*iter != '[') {
            json_parsing_error(iter, "No tuple start token");
        }
        ++iter;

        into.write();
        return detail::TupleHandler<std::tuple<Types...>,
                                    0,
                                    sizeof...(Types) == 1,
                                    Types...
                                   >::from_json(iter, into);
    }
} /* tjson */
#endif
