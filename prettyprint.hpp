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

#if __cplusplus >= 201103L // we are using a compiler with c++11

#include <cstddef>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <valarray>

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
              typename TChar = char,
              typename TCharTraits = ::std::char_traits<TChar>,
              typename TDelimiters = delimiters<T, TChar>>
    struct print_container_helper
    {
        using delimiters_type = TDelimiters;
        using ostream_type = std::basic_ostream<TChar, TCharTraits>;

        template <typename U>
        struct printer
        {
            static void print_body(const U & c, ostream_type & stream)
            {
                using std::begin;
                using std::end;

                auto it = begin(c);
                const auto the_end = end(c);

                if (it != the_end)
                {
                    for ( ; ; )
                    {
                        stream << *it;

                    if (++it == the_end) break;

                    if (delimiters_type::values.delimiter != NULL)
                        stream << delimiters_type::values.delimiter;
                    }
                }
            }
        };

        print_container_helper(const T & container)
        : container_(container)
        { }

        inline void operator()(ostream_type & stream) const
        {
            if (delimiters_type::values.prefix != NULL)
                stream << delimiters_type::values.prefix;

            printer<T>::print_body(container_, stream);

            if (delimiters_type::values.postfix != NULL)
                stream << delimiters_type::values.postfix;
        }

    private:
        const T & container_;
    };

    // Specialization for pairs

    template <typename T, typename TChar, typename TCharTraits, typename TDelimiters>
    template <typename T1, typename T2>
    struct print_container_helper<T, TChar, TCharTraits, TDelimiters>::printer<std::pair<T1, T2>>
    {
        using ostream_type = typename print_container_helper<T, TChar, TCharTraits, TDelimiters>::ostream_type;

        static void print_body(const std::pair<T1, T2> & c, ostream_type & stream)
        {
            stream << c.first;
            if (print_container_helper<T, TChar, TCharTraits, TDelimiters>::delimiters_type::values.delimiter != NULL)
                stream << print_container_helper<T, TChar, TCharTraits, TDelimiters>::delimiters_type::values.delimiter;
            stream << c.second;
        }
    };

    // Specialization for tuples

    template <typename T, typename TChar, typename TCharTraits, typename TDelimiters>
    template <typename ...Args>
    struct print_container_helper<T, TChar, TCharTraits, TDelimiters>::printer<std::tuple<Args...>>
    {
        using ostream_type = typename print_container_helper<T, TChar, TCharTraits, TDelimiters>::ostream_type;
        using element_type = std::tuple<Args...>;

        template <std::size_t I> struct Int { };

        static void print_body(const element_type & c, ostream_type & stream)
        {
            tuple_print(c, stream, Int<0>());
        }

        static void tuple_print(const element_type &, ostream_type &, Int<sizeof...(Args)>)
        {
        }

        static void tuple_print(const element_type & c, ostream_type & stream,
                                typename std::conditional<sizeof...(Args) != 0, Int<0>, std::nullptr_t>::type)
        {
            stream << std::get<0>(c);
            tuple_print(c, stream, Int<1>());
        }

        template <std::size_t N>
        static void tuple_print(const element_type & c, ostream_type & stream, Int<N>)
        {
            if (print_container_helper<T, TChar, TCharTraits, TDelimiters>::delimiters_type::values.delimiter != NULL)
                stream << print_container_helper<T, TChar, TCharTraits, TDelimiters>::delimiters_type::values.delimiter;

            stream << std::get<N>(c);

            tuple_print(c, stream, Int<N + 1>());
        }
    };

    // Prints a print_container_helper to the specified stream.

    template<typename T, typename TChar, typename TCharTraits, typename TDelimiters>
    inline std::basic_ostream<TChar, TCharTraits> & operator<<(
        std::basic_ostream<TChar, TCharTraits> & stream,
        const print_container_helper<T, TChar, TCharTraits, TDelimiters> & helper)
    {
        helper(stream);
        return stream;
    }


    // Basic is_container template; specialize to derive from std::true_type for all desired container types

    template <typename T>
    struct is_container : public std::integral_constant<bool,
                                                        detail::has_const_iterator<T>::value &&
                                                        detail::has_begin_end<T>::beg_value  &&
                                                        detail::has_begin_end<T>::end_value> { };

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
    template <typename T1, typename T2> const delimiters_values<char> delimiters<std::pair<T1, T2>, char>::values = { "(", ", ", ")" };
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

    template <typename T, typename Delims>
    struct custom_delims_wrapper : custom_delims_base
    {
        custom_delims_wrapper(const T & t_) : t(t_) { }

        std::ostream & stream(std::ostream & s)
        {
            return s << print_container_helper<T, char, std::char_traits<char>, Delims>(t);
        }

        std::wostream & stream(std::wostream & s)
        {
            return s << print_container_helper<T, wchar_t, std::char_traits<wchar_t>, Delims>(t);
        }

    private:
        const T & t;
    };

    template <typename Delims>
    struct custom_delims
    {
        template <typename Container>
        custom_delims(const Container & c) : base(new custom_delims_wrapper<Container, Delims>(c)) { }

        std::unique_ptr<custom_delims_base> base;
    };

    template <typename TChar, typename TCharTraits, typename Delims>
    inline std::basic_ostream<TChar, TCharTraits> & operator<<(std::basic_ostream<TChar, TCharTraits> & s, const custom_delims<Delims> & p)
    {
        return p.base->stream(s);
    }


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


// Main magic entry point: An overload snuck into namespace std.
// Can we do better?

namespace std
{
    // Prints a container to the stream using default delimiters

    template<typename T, typename TChar, typename TCharTraits>
    inline typename enable_if< ::pretty_print::is_container<T>::value,
                              basic_ostream<TChar, TCharTraits> &>::type
    operator<<(basic_ostream<TChar, TCharTraits> & stream, const T & container)
    {
        return stream << ::pretty_print::print_container_helper<T, TChar, TCharTraits>(container);
    }
}

#else // we are using pre-c++11 compiler

#include <ostream>
#include <utility>
#include <iterator>
#include <set>

#ifndef NO_TR1
#  include <tr1/tuple>
#  include <tr1/unordered_set>
#endif

namespace pretty_print
{

    template <bool, typename S, typename T> struct conditional { };
    template <typename S, typename T> struct conditional<true,  S, T> { typedef S type; };
    template <typename S, typename T> struct conditional<false, S, T> { typedef T type; };

    template <bool, typename T> struct enable_if { };
    template <typename T> struct enable_if<true, T> { typedef T type; };

    // SFINAE type trait to detect whether T::const_iterator exists.

    template<typename T>
    struct has_const_iterator
    {
    private:
        typedef char                      yes;
        typedef struct { char array[2]; } no;

        template<typename C> static yes test(typename C::const_iterator*);
        template<typename C> static no  test(...);
    public:
        static const bool value = sizeof(test<T>(0)) == sizeof(yes);
        typedef T type;
    };

    // SFINAE type trait to detect whether "T::const_iterator T::begin/end() const" exist.

    template <typename T>
    struct has_begin_end
    {
        struct Dummy { typedef void const_iterator; };
        typedef typename conditional<has_const_iterator<T>::value, T, Dummy>::type TType;
        typedef typename TType::const_iterator iter;

        struct Fallback { iter begin() const; iter end() const; };
        struct Derived : TType, Fallback { };

        template<typename C, C> struct ChT;

        template<typename C> static char (&f(ChT<iter (Fallback::*)() const, &C::begin>*))[1];
        template<typename C> static char (&f(...))[2];
        template<typename C> static char (&g(ChT<iter (Fallback::*)() const, &C::end>*))[1];
        template<typename C> static char (&g(...))[2];

        static bool const beg_value = sizeof(f<Derived>(0)) == 2;
        static bool const end_value = sizeof(g<Derived>(0)) == 2;
    };

    // Basic is_container template; specialize to have value "true" for all desired container types

    template<typename T> struct is_container { static const bool value = has_const_iterator<T>::value && has_begin_end<T>::beg_value && has_begin_end<T>::end_value; };

    template<typename T, std::size_t N> struct is_container<T[N]> { static const bool value = true; };

    template<std::size_t N> struct is_container<char[N]> { static const bool value = false; };


    // Holds the delimiter values for a specific character type

    template<typename TChar>
    struct delimiters_values
    {
        typedef TChar char_type;
        const TChar * prefix;
        const TChar * delimiter;
        const TChar * postfix;
    };


    // Defines the delimiter values for a specific container and character type

    template<typename T, typename TChar>
    struct delimiters
    {
        typedef delimiters_values<TChar> type;
        static const type values; 
    };


    // Default delimiters

    template<typename T> struct delimiters<T, char> { static const delimiters_values<char> values; };
    template<typename T> const delimiters_values<char> delimiters<T, char>::values = { "[", ", ", "]" };
    template<typename T> struct delimiters<T, wchar_t> { static const delimiters_values<wchar_t> values; };
    template<typename T> const delimiters_values<wchar_t> delimiters<T, wchar_t>::values = { L"[", L", ", L"]" };


    // Delimiters for (multi)set and unordered_(multi)set

    template<typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::set<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };

    template<typename T, typename TComp, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::set<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };

    template<typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::set<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template<typename T, typename TComp, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::set<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    template<typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::multiset<T, TComp, TAllocator>, char> { static const delimiters_values<char> values; };

    template<typename T, typename TComp, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::multiset<T, TComp, TAllocator>, char>::values = { "{", ", ", "}" };

    template<typename T, typename TComp, typename TAllocator>
    struct delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template<typename T, typename TComp, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::multiset<T, TComp, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

#ifndef NO_TR1
    template<typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::tr1::unordered_set<T, THash, TEqual, TAllocator>, char> { static const delimiters_values<char> values; };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::tr1::unordered_set<T, THash, TEqual, TAllocator>, char>::values = { "{", ", ", "}" };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::tr1::unordered_set<T, THash, TEqual, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::tr1::unordered_set<T, THash, TEqual, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::tr1::unordered_multiset<T, THash, TEqual, TAllocator>, char> { static const delimiters_values<char> values; };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<char> delimiters< ::std::tr1::unordered_multiset<T, THash, TEqual, TAllocator>, char>::values = { "{", ", ", "}" };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    struct delimiters< ::std::tr1::unordered_multiset<T, THash, TEqual, TAllocator>, wchar_t> { static const delimiters_values<wchar_t> values; };

    template<typename T, typename THash, typename TEqual, typename TAllocator>
    const delimiters_values<wchar_t> delimiters< ::std::tr1::unordered_multiset<T, THash, TEqual, TAllocator>, wchar_t>::values = { L"{", L", ", L"}" };
#endif


    // Delimiters for pair (reused for tuple, see below)

    template<typename T1, typename T2> struct delimiters< ::std::pair<T1, T2>, char> { static const delimiters_values<char> values; };
    template<typename T1, typename T2> const delimiters_values<char> delimiters< ::std::pair<T1, T2>, char>::values = { "(", ", ", ")" };
    template<typename T1, typename T2> struct delimiters< ::std::pair<T1, T2>, wchar_t> { static const delimiters_values<wchar_t> values; };
    template<typename T1, typename T2> const delimiters_values<wchar_t> delimiters< ::std::pair<T1, T2>, wchar_t>::values = { L"(", L", ", L")" };


    // Iterator microtrait class to handle C arrays uniformly

    template <typename T> struct get_iterator { typedef typename T::const_iterator iter; };
    template <typename T, std::size_t N> struct get_iterator<T[N]> { typedef const T * iter; };

    template <typename T> typename enable_if<has_const_iterator<T>::value, typename T::const_iterator>::type begin(const T & c) { return c.begin(); }
    template <typename T> typename enable_if<has_const_iterator<T>::value, typename T::const_iterator>::type end(const T & c) { return c.end(); }
    template <typename T, size_t N> const T * begin(const T(&x)[N]) { return &x[0];     }
    template <typename T, size_t N> const T * end  (const T(&x)[N]) { return &x[0] + N; }

    // Functor to print containers. You can use this directly if you want to specificy a non-default delimiters type.

    template<typename T, typename TChar = char, typename TCharTraits = ::std::char_traits<TChar>, typename TDelimiters = delimiters<T, TChar> >
    struct print_container_helper
    {
        typedef TChar char_type;
        typedef TDelimiters delimiters_type;
        typedef std::basic_ostream<TChar, TCharTraits> ostream_type;
        typedef typename get_iterator<T>::iter TIter;

        print_container_helper(const T & container)
        : _container(container)
        {
        }

        inline void operator()(ostream_type & stream) const
        {
            if (delimiters_type::values.prefix != NULL)
                stream << delimiters_type::values.prefix;

            if (begin(_container) != end(_container))
            for (TIter it = begin(_container), it_end = end(_container); ; )
            {
                stream << *it;

                if (++it == it_end) break;

                if (delimiters_type::values.delimiter != NULL)
                    stream << delimiters_type::values.delimiter;
            }

            if (delimiters_type::values.postfix != NULL)
                stream << delimiters_type::values.postfix;
        }

    private:
        const T & _container;
    };


    // Type-erasing helper class for easy use of custom delimiters.
    // Requires TCharTraits = std::char_traits<TChar> and TChar = char or wchar_t, and MyDelims needs to be defined for TChar.
    // Usage: "cout << pretty_print::custom_delims<MyDelims>(x)".

    struct custom_delims_base
    {
        virtual ~custom_delims_base() { }
        virtual ::std::ostream & stream(::std::ostream &) = 0;
        virtual ::std::wostream & stream(::std::wostream &) = 0;
    };

    template<typename T, typename Delims>
    struct custom_delims_wrapper : public custom_delims_base
    {
        custom_delims_wrapper(const T & t_) : t(t_) { }

        ::std::ostream & stream(::std::ostream & s)
        {
          return s << ::pretty_print::print_container_helper<T, char, ::std::char_traits<char>, Delims>(t);
        }
        ::std::wostream & stream(::std::wostream & s)
        {
          return s << ::pretty_print::print_container_helper<T, wchar_t, ::std::char_traits<wchar_t>, Delims>(t);
        }

    private:
        const T & t;
    };

    template<typename Delims>
    struct custom_delims
    {
        template<typename Container> custom_delims(const Container & c) : base(new custom_delims_wrapper<Container, Delims>(c)) { }
        ~custom_delims() { delete base; }
        custom_delims_base * base;
    };

} // namespace pretty_print

template<typename TChar, typename TCharTraits, typename Delims>
inline std::basic_ostream<TChar, TCharTraits> & operator<<(std::basic_ostream<TChar, TCharTraits> & s, const pretty_print::custom_delims<Delims> & p)
{
    return p.base->stream(s);
}

// Template aliases for char and wchar_t delimiters
// Enable these if you have compiler support
//
// Implement as "template<T, C, A> const sdelims::type sdelims<std::set<T,C,A>>::values = { ... }."

//template<typename T> using pp_sdelims = pretty_print::delimiters<T, char>;
//template<typename T> using pp_wsdelims = pretty_print::delimiters<T, wchar_t>;


namespace std
{
    // Prints a print_container_helper to the specified stream.

    template<typename T, typename TChar, typename TCharTraits, typename TDelimiters>
    inline basic_ostream<TChar, TCharTraits> & operator<<(basic_ostream<TChar, TCharTraits> & stream,
                                                          const ::pretty_print::print_container_helper<T, TChar, TCharTraits, TDelimiters> & helper)
    {
        helper(stream);
        return stream;
    }

    // Prints a container to the stream using default delimiters

    template<typename T, typename TChar, typename TCharTraits>
    inline typename ::pretty_print::enable_if< ::pretty_print::is_container<T>::value, basic_ostream<TChar, TCharTraits>&>::type
    operator<<(basic_ostream<TChar, TCharTraits> & stream, const T & container)
    {
        return stream << ::pretty_print::print_container_helper<T, TChar, TCharTraits>(container);
    }

    // Prints a pair to the stream using delimiters from delimiters<std::pair<T1, T2>>.
    template<typename T1, typename T2, typename TChar, typename TCharTraits>
    inline basic_ostream<TChar, TCharTraits> & operator<<(basic_ostream<TChar, TCharTraits> & stream, const pair<T1, T2> & value)
    {
        if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.prefix != NULL)
            stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.prefix;

        stream << value.first;

        if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.delimiter != NULL)
            stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.delimiter;

        stream << value.second;

        if (::pretty_print::delimiters<pair<T1, T2>, TChar>::values.postfix != NULL)
            stream << ::pretty_print::delimiters<pair<T1, T2>, TChar>::values.postfix;

        return stream;
    }
} // namespace std


#ifndef NO_TR1

// Prints a tuple to the stream using delimiters from delimiters<std::pair<tuple_dummy_t, tuple_dummy_t>>.

namespace pretty_print
{
    struct tuple_dummy_t { }; // Just if you want special delimiters for tuples.

    typedef std::pair<tuple_dummy_t, tuple_dummy_t> tuple_dummy_pair;

    template<typename Tuple, size_t N, typename TChar, typename TCharTraits>
    struct pretty_tuple_helper
    {
        static inline void print(::std::basic_ostream<TChar, TCharTraits> & stream, const Tuple & value)
        {
            pretty_tuple_helper<Tuple, N - 1, TChar, TCharTraits>::print(stream, value);

            if (delimiters<tuple_dummy_pair, TChar>::values.delimiter != NULL)
                stream << delimiters<tuple_dummy_pair, TChar>::values.delimiter;

            stream << std::tr1::get<N - 1>(value);
        }
    };

    template<typename Tuple, typename TChar, typename TCharTraits>
    struct pretty_tuple_helper<Tuple, 1, TChar, TCharTraits>
    {
        static inline void print(::std::basic_ostream<TChar, TCharTraits> & stream, const Tuple & value)
        {
            stream << ::std::tr1::get<0>(value);
        }
    };
} // namespace pretty_print


/* The following macros allow us to write "template <TUPLE_PARAMAS> std::tuple<TUPLE_ARGS>"
 * uniformly in C++0x compilers and in MS Visual Studio 2010.
 * Credits to STL: http://channel9.msdn.com/Shows/Going+Deep/C9-Lectures-Stephan-T-Lavavej-Advanced-STL-6-of-n
 */

#define TUPLE_PARAMS \
    typename T0, typename T1, typename T2, typename T3, typename T4,      \
    typename T5, typename T6, typename T7, typename T8, typename T9
#define TUPLE_ARGS T0, T1, T2, T3, T4, T5, T6, T7, T8, T9


namespace std
{
    template<typename TChar, typename TCharTraits, TUPLE_PARAMS>
    inline basic_ostream<TChar, TCharTraits> & operator<<(basic_ostream<TChar, TCharTraits> & stream, const tr1::tuple<TUPLE_ARGS> & value)
    {
        if (::pretty_print::delimiters< ::pretty_print::tuple_dummy_pair, TChar>::values.prefix != NULL)
            stream << ::pretty_print::delimiters< ::pretty_print::tuple_dummy_pair, TChar>::values.prefix;

        ::pretty_print::pretty_tuple_helper<const tr1::tuple<TUPLE_ARGS> &, tr1::tuple_size<tr1::tuple<TUPLE_ARGS> >::value, TChar, TCharTraits>::print(stream, value);

        if (::pretty_print::delimiters< ::pretty_print::tuple_dummy_pair, TChar>::values.postfix != NULL)
            stream << ::pretty_print::delimiters< ::pretty_print::tuple_dummy_pair, TChar>::values.postfix;

        return stream;
    }
} // namespace std

#endif // NO_TR1


// A wrapper for raw C-style arrays. Usage: int arr[] = { 1, 2, 4, 8, 16 };  std::cout << wrap_array(arr) << ...

namespace pretty_print
{
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
} // namespace pretty_print

template<typename T>
inline pretty_print::array_wrapper_n<T> pretty_print_array(const T * const a, size_t n)
{
  return pretty_print::array_wrapper_n<T>(a, n);
}

#endif // c++11 or not

#endif // H_PRETTY_PRINT

