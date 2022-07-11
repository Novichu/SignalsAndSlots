#ifndef __XSINGLETON__HPP__
#define __XSINGLETON__HPP__
#include <type_traits> // std::is_class_v<T>
#include <mutex>       // std::once_flag
#include <memory>      //std::unique_ptr
// How To Use Only
///
// class xxxxx : public XSingleton<xxxxx>, XNoCopy
// {
// };
// xxxxx &b = xxxxx::Instance();
///
namespace Novichu
{
    template <typename ThisType>
    requires std::is_class_v<ThisType>
    class XSingleton
    {
    public:
        template <typename... Args>
        static ThisType &Instance(Args &&...args)
        {
            static auto _instance = ThisType{std::forward<Args>(args)...};
            return _instance;
        }
    };
}
#endif