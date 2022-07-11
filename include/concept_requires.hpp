#ifndef __CONCEPT_REQUIRES__HPP__
#define __CONCEPT_REQUIRES__HPP__
#include <string>
#include <tuple>
#include <concepts>
#include <chrono>
namespace Novichu
{
    template <typename T>
    concept digital_checker = std::integral<T> || std::floating_point<T>;
    template <typename T>
    concept string_checker = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v <std::remove_cvref_t< std::decay_t<T>>,char * > || std::is_same_v <std::remove_cvref_t< std::decay_t<T>>,const char * >;
    template <typename T>
    concept container_checker = requires(T t)
    {
        std::ranges::begin(t);
        std::ranges::end(t);
        requires !string_checker<T>;
    };
    template <typename T>
    concept tuple_checker = !std::is_reference_v<T> && requires(T t)
    {
        typename std::tuple_size<T>::type;
        requires std::derived_from<
            std::tuple_size<T>,
            std::integral_constant<std::size_t, std::tuple_size_v<T>>
            >;
    };
};
#endif
