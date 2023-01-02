//          Copyright Louis Delacroix 2010 - 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// A pretty printing library for C++
//
// Usage:
// Include this header, and operator<< will "just work".

#ifndef H_PRETTY_PRINT
#define H_PRETTY_PRINT

#include <cstddef>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <sstream>

template <typename T>
std::string ToString(const T& container);

namespace pretty_print
{
    namespace detail
    {
        // SFINAE type trait to detect whether T::const_iterator exists.

        struct sfinae_base
        {
            using yes = char;
            using no  = yes[2];
        };

        template <typename T>
        struct has_const_iterator : private sfinae_base
        {
        private:
            template <typename C> static yes & test(typename C::const_iterator*);
            template <typename C> static no  & test(...);
        public:
            // 判断 T 是否有 const_iterator
            static const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
            using type =  T;
        };

        template <typename T>
        struct has_begin_end : private sfinae_base
        {
        private:
            template <typename C>
            static yes & f(typename std::enable_if<
                std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::begin)),
                             typename C::const_iterator(C::*)() const>::value>::type *);

            template <typename C> static no & f(...);

            template <typename C>
            static yes & g(typename std::enable_if<
                std::is_same<decltype(static_cast<typename C::const_iterator(C::*)() const>(&C::end)),
                             typename C::const_iterator(C::*)() const>::value, void>::type*);

            template <typename C> static no & g(...);

        public:
            static bool const beg_value = sizeof(f<T>(nullptr)) == sizeof(yes);
            static bool const end_value = sizeof(g<T>(nullptr)) == sizeof(yes);
        };

    }  // namespace detail


    // Holds the delimiter values for a specific character type

    template <typename TChar>
    struct delimiters_values
    {
        using char_type = TChar;
        const char_type * prefix;
        const char_type * delimiter;
        const char_type * postfix;
    };


    // Defines the delimiter values for a specific container and character type

    template <typename T, typename TChar>
    struct delimiters
    {
        using type = delimiters_values<TChar>;
        static const type values;
    };


    // Functor to print containers. You can use this directly if you want
    // to specificy a non-default delimiters type. The printing logic can
    // be customized by specializing the nested template.

    template <typename T,
              typename TDelimiters = delimiters<T, char>>
    struct print_container_helper
    {
        using delimiters_type = TDelimiters;

        template <typename U>
        struct printer
        {
            static std::string print_body(const U & c)
            {
                std::string r;
                bool is_first = true;
                for (auto& ele : c) {
                    if (delimiters_type::values.delimiter != NULL && !is_first) {
                        r += delimiters_type::values.delimiter;
                    }
                    r += ToString(ele);
                    is_first = false;
                }
                return r;
            }
        };

        print_container_helper(const T & container)
        : container_(container)
        { }

        inline std::string operator()() const
        {
            std::string r;
            if (delimiters_type::values.prefix != NULL)
                r += delimiters_type::values.prefix;

            r += printer<T>::print_body(container_);

            if (delimiters_type::values.postfix != NULL)
                r+= delimiters_type::values.postfix;
            return r;
        }

    private:
        const T & container_;
    };
    
    // Specialization for pairs

    template <typename T, typename TDelimiters>
    template <typename T1, typename T2>
    struct print_container_helper<T, TDelimiters>::printer<std::pair<T1, T2>>
    {
        static std::string print_body(const std::pair<T1, T2> & c)
        {
            std::string s;
            s += ToString(c.first);
            if (print_container_helper<T, TDelimiters>::delimiters_type::values.delimiter != NULL)
                s += print_container_helper<T, TDelimiters>::delimiters_type::values.delimiter;
            s += ToString(c.second);
            return s;
        }
    };


    // Specialization for tuples

    template <typename T, typename TDelimiters>
    template <typename ...Args>
    struct print_container_helper<T, TDelimiters>::printer<std::tuple<Args...>>
    {
        using element_type = std::tuple<Args...>;

        template <std::size_t I> struct Int { };

        static std::string print_body(const element_type & c)
        {
            return tuple_print(c, Int<0>());
        }

        static std::string tuple_print(const element_type &, Int<sizeof...(Args)>)
        {
            return "";
        }

        static std::string tuple_print(const element_type & c,
                                typename std::conditional<sizeof...(Args) != 0, Int<0>, std::nullptr_t>::type)
        {
            std::string r;
            r += ToString(std::get<0>(c));
            r += tuple_print(c, Int<1>());
            return r;
        }

        template <std::size_t N>
        static std::string tuple_print(const element_type & c, Int<N>)
        {
            std::string r;
            if (print_container_helper<T, TDelimiters>::delimiters_type::values.delimiter != NULL)
                r +=  print_container_helper<T, TDelimiters>::delimiters_type::values.delimiter;

            r += ToString(std::get<N>(c));

            r += tuple_print(c, Int<N + 1>());
            return r;
        }
    };

    // Basic is_string_like template; specialize to derive from std::true_type for all desired container types
    // 基础的 is_string_like 模板，用于判断类型 T 是否是可以被 std::string 构造的。
    template <typename T,
            typename T0 = typename std::decay<T>::type,
			typename T1 = typename std::remove_pointer<T0>::type,
			typename T2 = typename std::remove_const<T1>::type,
			typename T3 = typename std::add_pointer<T2>::type
	>
    struct is_string_like : public std::integral_constant<bool,
                                                        std::is_same<T, std::string>::value ||
                                                        std::is_same<T3, char*>::value
                                                        > { };
    
    // Basic is_container template; specialize to derive from std::true_type for all desired container types

    template <typename T>
    struct is_container : public std::integral_constant<bool,
                                                        detail::has_const_iterator<T>::value &&
                                                        detail::has_begin_end<T>::beg_value  &&
                                                        detail::has_begin_end<T>::end_value  &&
                                                        !is_string_like<T>::value
                                                        > { };

    template <typename T, std::size_t N>
    struct is_container<T[N]> : std::true_type { };

    template <std::size_t N>
    struct is_container<char[N]> : std::false_type { };

    template <typename T>
    struct is_container<std::valarray<T>> : std::true_type { };

    template <typename T1, typename T2>
    struct is_container<std::pair<T1, T2>> : std::true_type { };

    template <typename ...Args>
    struct is_container<std::tuple<Args...>> : std::true_type { };


    // Default delimiters

    template <typename T> struct delimiters<T, char> { static const delimiters_values<char> values; };
    template <typename T> const delimiters_values<char> delimiters<T, char>::values = { "[", ", ", "]" };
    template <typename T> struct delimiters<T, wchar_t> { static const delimiters_values<wchar_t> values; };
    template <typename T> const delimiters_values<wchar_t> delimiters<T, wchar_t>::values = { L"[", L", ", L"]" };


    // Delimiters for (multi)set and unordered_(multi)set

    template <typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::set<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };

    template <typename T, typename TComp, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::set<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };

    template <typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::set<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template <typename T, typename TComp, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::set<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    template <typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::multiset<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };

    template <typename T, typename TComp, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::multiset<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };

    template <typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template <typename T, typename TComp, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::unordered_set<T, THash, TEqual, TAllocator>, char> { static const delimiters_values<char> values; };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::unordered_set<T, THash, TEqual, TAllocator>, char>::values = { "{", ", ", "}" };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::unordered_set<T, THash, TEqual, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::unordered_set<T, THash, TEqual, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::unordered_multiset<T, THash, TEqual, TAllocator>, char> { static const delimiters_values<char> values; };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::unordered_multiset<T, THash, TEqual, TAllocator>, char>::values = { "{", ", ", "}" };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::unordered_multiset<T, THash, TEqual, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template <typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::unordered_multiset<T, THash, TEqual, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    // Delimiters for pair and tuple

    template <typename T1, typename T2> struct delimiters<std::pair<T1, T2>, char> { static const delimiters_values<char> values; };
    template <typename T1, typename T2> const delimiters_values<char> delimiters<std::pair<T1, T2>, char>::values = { "", ": ", "" };
    template <typename T1, typename T2> struct delimiters< ::std::pair<T1, T2>, wchar_t> { static const delimiters_values<wchar_t> values; };
    template <typename T1, typename T2> const delimiters_values<wchar_t> delimiters< ::std::pair<T1, T2>, wchar_t>::values = { L"(", L", ", L")" };

    template <typename ...Args> struct delimiters<std::tuple<Args...>, char> { static const delimiters_values<char> values; };
    template <typename ...Args> const delimiters_values<char> delimiters<std::tuple<Args...>, char>::values = { "(", ", ", ")" };
    template <typename ...Args> struct delimiters< ::std::tuple<Args...>, wchar_t> { static const delimiters_values<wchar_t> values; };
    template <typename ...Args> const delimiters_values<wchar_t> delimiters< ::std::tuple<Args...>, wchar_t>::values = { L"(", L", ", L")" };


    // Type-erasing helper class for easy use of custom delimiters.
    // Requires TCharTraits = std::char_traits<TChar> and TChar = char or wchar_t, and MyDelims needs to be defined for TChar.
    // Usage: "cout << pretty_print::custom_delims<MyDelims>(x)".

    struct custom_delims_base
    {
        virtual ~custom_delims_base() { }
        virtual std::ostream & stream(::std::ostream &) = 0;
        virtual std::wostream & stream(::std::wostream &) = 0;
    };

    // A wrapper for a C-style array given as pointer-plus-size.
    // Usage: std::cout << pretty_print_array(arr, n) << std::endl;

    template<typename T>
    struct array_wrapper_n
    {
        typedef const T * const_iterator;
        typedef T value_type;

        array_wrapper_n(const T * const a, size_t n) : _array(a), _n(n) { }
        inline const_iterator begin() const { return _array; }
        inline const_iterator end() const { return _array + _n; }

    private:
        const T * const _array;
        size_t _n;
    };


    // A wrapper for hash-table based containers that offer local iterators to each bucket.
    // Usage: std::cout << bucket_print(m, 4) << std::endl;  (Prints bucket 5 of container m.)

    template <typename T>
    struct bucket_print_wrapper
    {
        typedef typename T::const_local_iterator const_iterator;
        typedef typename T::size_type size_type;

        const_iterator begin() const
        {
            return m_map.cbegin(n);
        }

        const_iterator end() const
        {
            return m_map.cend(n);
        }

        bucket_print_wrapper(const T & m, size_type bucket) : m_map(m), n(bucket) { }

    private:
        const T & m_map;
        const size_type n;
    };

}   // namespace pretty_print


// Global accessor functions for the convenience wrappers

template<typename T>
inline pretty_print::array_wrapper_n<T> pretty_print_array(const T * const a, size_t n)
{
    return pretty_print::array_wrapper_n<T>(a, n);
}

template <typename T> pretty_print::bucket_print_wrapper<T>
bucket_print(const T & m, typename T::size_type n)
{
    return pretty_print::bucket_print_wrapper<T>(m, n);
}


template <typename T>
std::string ToStringHelper(const T& container,
    typename std::enable_if<::pretty_print::is_container<T>::value>::type* = 0,
    typename std::enable_if<!::pretty_print::is_string_like<T>::value>::type* = 0
    ) {
    std::string r = ::pretty_print::print_container_helper<T>(container)();
    return r;
}


template <typename T>
std::string ToStringHelper(const T& container,
    typename std::enable_if<!::pretty_print::is_container<T>::value>::type* = 0,
    typename std::enable_if<!::pretty_print::is_string_like<T>::value>::type* = 0
    ) {
    return std::to_string(container);
}

template <typename T>
std::string ToStringHelper(const T& s,
    typename std::enable_if<!::pretty_print::is_container<T>::value>::type* = 0,
    typename std::enable_if<::pretty_print::is_string_like<T>::value>::type* = 0
    ) {
    return "\"" + std::string(s) + "\"";
}

template <typename T>
std::string ToString(const T& container) {
    // using namespace std;
    // cout << pretty_print::is_string_like<T>::value << endl;
    // cout << pretty_print::is_container<T>::value << endl;
    // return "";
    return ToStringHelper(container, nullptr, nullptr);
}

#endif  // H_PRETTY_PRINT
