#pragma once
#ifndef __JSON_PRIVATE_ACCESS_HPP__
#define __JSON_PRIVATE_ACCESS_HPP__

#include "json_functions.hpp"

#define JSON_PRIVATE_ACCESS()                               \
    template<typename ClassFor>                             \
    friend JSON::stringt JSON::ToJSON(const ClassFor&);     \
                                                            \
    template<typename ClassFor>                             \
    friend ClassFor JSON::FromJSON(const JSON::stringt&);   \
                                                            \
    template<typename ClassFor>                             \
    friend JSON::jsonIter JSON::FromJSON(JSON::jsonIter, JSON::jsonIter, ClassFor&);

#endif