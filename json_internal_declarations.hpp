#pragma once
#ifndef __JSON_INTERNAL_DECLARATIONS__
#define __JSON_INTERNAL_DECLARATIONS__

#include <type_traits>

namespace tjson {
namespace detail {
        template<typename>
        struct DataMember;

        template<typename ClassType>
        using non_const = typename std::remove_const<ClassType>::type;

        template<typename ClassType>
        using basic_type = non_const<typename std::remove_reference<ClassType>::type>;

        template<typename ClassType, template<typename C> class decider>
        using enable_if = typename std::enable_if<decider<basic_type<ClassType>>::value, bool>::type;

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        inline void to_json(ClassType from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_enum> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        inline void to_json(ClassType from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_integral> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        inline void to_json(ClassType from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_floating_point> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        inline void to_json(ClassType from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_pointer> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        inline void to_json(const ClassType& from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_array> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        inline void to_json(const ClassType& from, detail::Stringbuf& out);

        template<typename ClassType, enable_if<ClassType, std::is_class> = true>
        inline jsonIter from_json(jsonIter iter, DataMember<ClassType>& into);

} /* detail */
} /* tjson */
#endif
