
#ifndef __XTRAITS__HPP__
#define __XTRAITS__HPP__
#include <type_traits> // std::is_invocable_r_v  std::is_invocable_v
#include <tuple>
namespace Novichu
{
    template <typename T>
    struct function_traits_helper
    {
        using type = T;
    };
    template <class Obj, typename Ret, typename... Args>
    struct function_traits_helper<Ret (Obj::*)(Args...)>
    {
        using object = Obj;
        using return_type = Ret;
        using args_type = std::tuple<Args...>;
        static constexpr int args_count = sizeof...(Args);
        static constexpr bool value = true;
    };
    template <class Obj, typename Ret, typename... Args>
    struct function_traits_helper<Ret (Obj::*)(Args...) const>
    {
        using object = Obj;
        using return_type = Ret;
        using args_type = std::tuple<Args...>;
        static constexpr int args_count = sizeof...(Args);
        static constexpr bool value = true;
    };
    template <typename Ret, typename... Args>
    struct function_traits_helper<Ret(Args...)>
    {
        using return_type = Ret;
        using args_type = std::tuple<Args...>;
        static constexpr int args_count = sizeof...(Args);
        static constexpr bool value = false;
    };

    template <typename T>
    struct function_traits : public function_traits_helper<T>
    {
        using type = T;
    };
    template <typename T>
    requires std::is_class_v<T>
    struct function_traits<T> : public function_traits_helper<decltype(&T::operator())>
    {
        using type = T;
        static constexpr bool value = false;
    };
    template <class Obj, typename Ret, typename... Args>
    struct function_traits<Ret (Obj::*)(Args...)> : public function_traits_helper<Ret (Obj::*)(Args...)>
    {
        using type = Ret (Obj::*)(Args...);
    };
    template <class Obj, typename Ret, typename... Args>
    struct function_traits<Ret (Obj::*)(Args...) const> : public function_traits_helper<Ret (Obj::*)(Args...) const>
    {
        using type = Ret (Obj::*)(Args...) const;
    };
    template <typename Ret, typename... Args>
    struct function_traits<Ret(Args...)> : public function_traits_helper<Ret(Args...)>
    {
        using type = Ret(Args...);
    };
    template <typename Ret, typename... Args>
    struct function_traits<Ret (*)(Args...)> : public function_traits_helper<Ret(Args...)>
    {
        using type = Ret (*)(Args...);
    };
    template <typename Ret, typename... Args>
    struct function_traits<Ret (&)(Args...)> : public function_traits_helper<Ret(Args...)>
    {
        using type = Ret (&)(Args...);
    };
    template <typename Fn1, typename Fn2>
    concept is_args_count_same_v = function_traits<Fn1>::args_count == function_traits<Fn2>::args_count;
}
#endif
