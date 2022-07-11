#ifndef __XSERIALIZER__HPP__
#define __XSERIALIZER__HPP__
#include "XBuffer.hpp" // XBuffer
#include "concept_requires.hpp"

// How To Use Only
/// partial specialization XSerializer::output_type<T> or use XSerializerRegisterSType(type,classmember)
/// partial specialization XSerializer::input_type<T> or use XSerializerRegisterSType(type,classmember)
/// for example

// struct Data
// {
//     Data() {}
//     int b = 0;
//     int c = 0;
//     std::string s;
//     Data(int b, int c, const std::string &s) : b(b), c(c), s(s) {}
//     Data(int b, int c, std::string &&s) : b(b), c(c), s(std::move(s)) {}
//     Data(const Data &d) : b(d.b), c(d.c), s(d.s) {}
//     Data(Data &&d) : b(d.b), c(d.c), s(std::move(d.s)) {}
//     auto &operator=(const Data &d)
//     {
//         b = d.b, c = d.c, s = d.s;
//         return *this;
//     }
//     auto &operator=(Data &&d)
//     {
//         b = d.b, c = d.c, s = std::move(d.s);
//         d.b = 0;
//         d.c = 0;
//         return *this;
//     }
// };

///_____________way 1 __________________
// template <>
// inline void XSerializer::output_type<Data>(Data &t)
// {
//     output_type(t.b);
//     output_type(t.c);
//     output_type(t.s);
// }
// template <>
// inline void XSerializer::input_type<Data>(Data &t)
// {
//     input_type(t.b);
//     input_type(t.c);
//     input_type(t.s);
// }

///_____________way 2 __________________
// XSerializerRegisterSType(Data, b, c, s);

namespace Novichu
{
    template <typename Type>
    struct xserializer_helper; // xserializer_helper help to pack or unpack a given Type
    class XSerializer
    {
    public:
        XSerializer(/* args */) {}
        XSerializer(XBuffer &&buf) : _iodevice(std::move(buf)) {}
        XSerializer(const XBuffer &buf) : _iodevice(buf) {}
        ~XSerializer() {}
        void reset() { return _iodevice.reset(); }
        size_t size() { return _iodevice.size(); }
        void skip_raw_data(size_t k) { return _iodevice.offset(k); }
        std::string &data() { return _iodevice.data(); }
        void byte_orser(std::string &in)
        {
            if constexpr (!_byteorder)
                return std::reverse(in.begin(), in.end());
        }
        void write_raw_data(std::string &in)
        {
            _iodevice.input(in);
            _iodevice.offset(in.size());
        }
        std::string current(size_t len = std::string::npos) { return _iodevice.current(len); }
        void clear() { return _iodevice.clear(); }
        template <typename T>
        void output_type(T &t);
        template <typename T>
        void input_type(T &t);
        std::string get_length_buf(size_t len)
        {
            std::string buf = _iodevice.current(len);
            _iodevice.offset(len);
            return buf;
        }
        template <typename T>
        XSerializer &operator>>(T &&t)
        {
            output_type(t);
            return *this;
        }
        template <typename T>
        XSerializer &operator<<(T &&t)
        {
            input_type(t);
            return *this;
        }
        template <typename Tuple, std::size_t id>
        requires tuple_checker<Tuple>
        void getv(XSerializer &xs, Tuple &t) { xs >> std::get<id>(t); }

        template <typename Tuple, std::size_t... I>
        requires tuple_checker<std::remove_cvref_t<Tuple>>
        decltype(auto) get_tuple(std::index_sequence<I...>)
        {
            using tuple_type = std::remove_cvref_t<Tuple>;
            tuple_type t;
            ((getv<tuple_type, I>(*this, t)), ...);
            return t;
        }

    private:
        XBuffer _iodevice;
        static constexpr bool _byteorder = static_cast<char>(0x00000010); // little_end
    };
    template <typename... Args>
    requires tuple_checker<std::tuple<Args...>>
    void package_params(XSerializer &ds, std::tuple<Args...> &&t)
    {
        auto package_params_impl = [&ds]<size_t... idx>(std::tuple<Args...> && t, std::index_sequence<idx...>)
        {
            ((ds << std::get<idx>(std::forward<std::tuple<Args...>>(t))), ...);
        };
        package_params_impl(std::forward<std::tuple<Args...>>(t), std::index_sequence_for<Args...>{});
    }
#define XSerializerMakePartialSpecializationForFundamentalType(Type)   \
    template <>                                                        \
    inline void                                                        \
    XSerializer::output_type(Type &t)                                  \
    {                                                                  \
        constexpr size_t len = sizeof(std::decay_t<Type>);             \
        if (!_iodevice.is_end())                                       \
        {                                                              \
            std::string d = get_length_buf(len);                       \
            byte_orser(d);                                             \
            t = *reinterpret_cast<std::decay_t<Type> *>(&d.data()[0]); \
        }                                                              \
    }                                                                  \
    template <>                                                        \
    inline void XSerializer::input_type(Type &t)                       \
    {                                                                  \
        constexpr size_t len = sizeof(std::decay_t<Type>);             \
        std::string d{reinterpret_cast<const char *>(&t), len};        \
        byte_orser(d);                                                 \
        _iodevice.input(std::move(d));                                 \
    }
    XSerializerMakePartialSpecializationForFundamentalType(bool);
    XSerializerMakePartialSpecializationForFundamentalType(char);
    XSerializerMakePartialSpecializationForFundamentalType(signed char);
    XSerializerMakePartialSpecializationForFundamentalType(unsigned char);
    XSerializerMakePartialSpecializationForFundamentalType(wchar_t);
    XSerializerMakePartialSpecializationForFundamentalType(char8_t);
    XSerializerMakePartialSpecializationForFundamentalType(char16_t);
    XSerializerMakePartialSpecializationForFundamentalType(char32_t);
    XSerializerMakePartialSpecializationForFundamentalType(short);
    XSerializerMakePartialSpecializationForFundamentalType(unsigned short);
    XSerializerMakePartialSpecializationForFundamentalType(int);
    XSerializerMakePartialSpecializationForFundamentalType(unsigned int);
    XSerializerMakePartialSpecializationForFundamentalType(long);
    XSerializerMakePartialSpecializationForFundamentalType(unsigned long);
    XSerializerMakePartialSpecializationForFundamentalType(long long);
    XSerializerMakePartialSpecializationForFundamentalType(unsigned long long);
    XSerializerMakePartialSpecializationForFundamentalType(float);
    XSerializerMakePartialSpecializationForFundamentalType(double);
    XSerializerMakePartialSpecializationForFundamentalType(long double);
#undef XSerializerMakePartialSpecializationForFundamentalType

    template <>
    inline void XSerializer::output_type<std::string>(std::string &t)
    {
        constexpr size_t marklen = sizeof(uint16_t); // two bytes / string's size
        std::string d = get_length_buf(marklen);
        byte_orser(d);
        uint16_t len = *reinterpret_cast<uint16_t *>(&d.data()[0]);
        if (len != 0)
        {
            t = get_length_buf(len);
        }
    }
    template <>
    inline void XSerializer::input_type<std::string>(std::string &t)
    {
        uint16_t len = 0;
        std::string d{std::forward<std::string>(t)};
        // save  string's size
        len = (uint16_t)d.size(); // may been size < 65535
        std::string head{reinterpret_cast<char *>(&len), sizeof(uint16_t)};
        byte_orser(head);
        _iodevice.input(std::move(head));
        // save  string'self
        if (len != 0)
        {
            byte_orser(d);
            _iodevice.input(std::move(d));
        }
    }
#define XSerializerRegisterSTypeNoNPS(Type, ...)                       \
    template <>                                                        \
    struct xserializer_helper<Type> : public Type                      \
    {                                                                  \
        void pack(XSerializer &xs)                                     \
        {                                                              \
            pack_helper(xs, ##__VA_ARGS__);                            \
        }                                                              \
        void unpack(XSerializer &xs)                                   \
        {                                                              \
            unpack_helper(xs, ##__VA_ARGS__);                          \
        }                                                              \
        template <typename... ClassMember>                             \
        void pack_helper(XSerializer &xs, ClassMember &&...args)       \
        {                                                              \
            ((xs.input_type(args)), ...);                              \
        }                                                              \
        template <typename... ClassMember>                             \
        void unpack_helper(XSerializer &xs, ClassMember &&...args)     \
        {                                                              \
            ((xs.output_type(args)), ...);                             \
        }                                                              \
    };                                                                 \
    template <>                                                        \
    inline void XSerializer::output_type<Type>(Type & t)               \
    {                                                                  \
        reinterpret_cast<xserializer_helper<Type> &>(t).unpack(*this); \
    }                                                                  \
    template <>                                                        \
    inline void XSerializer::input_type<Type>(Type & t)                \
    {                                                                  \
        reinterpret_cast<xserializer_helper<Type> &>(t).pack(*this);   \
    }
#define XSerializerRegisterSType(Type, ...)                 \
    namespace Novichu                                       \
    {                                                       \
        XSerializerRegisterSTypeNoNPS(Type, ##__VA_ARGS__); \
    };
}
#endif
