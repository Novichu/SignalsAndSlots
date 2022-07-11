#ifndef __XCONNECT__HPP__
#define __XCONNECT__HPP__
#include <thread> // std::thread::id
namespace Novichu
{
    enum class XConnectType
    {
        AutoConnection,
        DirectConnection,
        QueuedConnection
    };
    struct XConnect
    {
        uint64_t obj1name;
        uint64_t fn1name;
        uint64_t obj2name;
        uint64_t fn2id;
        XConnectType contype;
        std::thread::id tid;
        void *obj_ptr = nullptr;
    };
// define X_XObject
#define X_XObject                                                                                   \
    template <typename Obj>                                                                         \
    requires std::derived_from<Obj, XObject>                                                        \
    static inline uint64_t _get_obj_address(Obj *obj)                                               \
    {                                                                                               \
        return reinterpret_cast<uint64_t>((XObject *)obj);                                          \
    };                                                                                              \
    template <typename Fn>                                                                          \
    requires function_traits<Fn>::value static inline uint64_t _get_function_value(Fn &fn) noexcept \
    {                                                                                               \
        union                                                                                       \
        {                                                                                           \
            Fn func;                                                                                \
            uint64_t value;                                                                         \
        };                                                                                          \
        func = *std::addressof(fn);                                                                 \
        return value;                                                                               \
    }; // a member pointer 16UL
// emit  a signal
#define X_Emit
// make  a signal
#define XMakeSignal(fn, ...)               \
    {                                      \
        active(this, &fn, ##__VA_ARGS__); \
    }

}
#endif