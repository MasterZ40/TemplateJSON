#pragma once
#ifndef __JSON_TUPLE_PARSER_HPP__
#define __JSON_TUPLE_PARSER_HPP__

/* I want to include as few headers as possible, but tuples suck to set up properly */
#include <tuple>

namespace JSON {
    template<typename TupleType,
             size_t curIndex,
             bool lastValue,
             typename curType,
             typename... Types>
    struct TupleHandler {
        json_finline static void ToJSON(const TupleType& classFrom, detail::stringbuf& out) {
            detail::ToJSON(std::get<curIndex>(classFrom), out);
            out.push_back(',');
            TupleHandler<TupleType,
                         curIndex + 1,
                         sizeof...(Types) == 1,
                         Types...
                        >::ToJSON(classFrom, out);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, TupleType& into) {
            curType& value = std::get<curIndex>(into);

            iter = detail::FromJSON(iter, value);
            iter = AdvancePastWhitespace(iter);
            if(*iter != ',') {
                ThrowBadJSONError(iter, "Not a valid tuple value");
            }
            ++iter;
            return TupleHandler<TupleType,
                                curIndex + 1,
                                sizeof...(Types) == 1,
                                Types...
                               >::FromJSON(iter, into);
        }
    };

    template<typename TupleType,
             size_t curIndex,
             typename curType,
             typename... Types>
    struct TupleHandler <TupleType,
                         curIndex,
                         true,
                         curType,
                         Types...> {
        json_finline static void ToJSON(const TupleType& classFrom, detail::stringbuf& out) {
            detail::ToJSON(std::get<curIndex>(classFrom), out);
        }

        json_finline static jsonIter FromJSON(jsonIter iter, TupleType& into) {
            curType& value = std::get<curIndex>(into);
            iter = detail::FromJSON(iter, value);
            iter = AdvancePastWhitespace(iter);
            if(*iter != ']') {
                ThrowBadJSONError(iter, "No tuple end token");
            }
            ++iter;
            return iter;
        }
    };

    template<typename... Types>
    void ToJSON(const std::tuple<Types...>& from, detail::stringbuf& out) {
        out.push_back('[');
        TupleHandler<std::tuple<Types...>,
                     0,
                     sizeof...(Types) == 1,
                     Types...
                    >::ToJSON(from, out);
        out.push_back(']');
    }

    template<typename... Types>
    jsonIter FromJSON(jsonIter iter, std::tuple<Types...>& into) {
        if(*iter != '[') {
            ThrowBadJSONError(iter, "No tuple start token");
        }
        ++iter;

        return TupleHandler<std::tuple<Types...>,
                            0,
                            sizeof...(Types) == 1,
                            Types...
                           >::FromJSON(iter, into);
    }
}
#endif
