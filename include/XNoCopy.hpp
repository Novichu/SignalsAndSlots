#ifndef __XNOCOPY__HPP__
#define __XNOCOPY__HPP__
namespace Novichu
{
    class XNoCopy
    {
    public:
        XNoCopy() = default;
        XNoCopy(const XNoCopy &) = delete;
        XNoCopy(XNoCopy &&) = delete;
        XNoCopy &operator=(const XNoCopy &) = delete;
        XNoCopy &operator=(XNoCopy &&) = delete;
    };
}
#endif