
#ifndef __XAPPLICATION__HPP__
#define __XAPPLICATION__HPP__
#include <functional>         // function
#include <thread>             // thread
#include <queue>              // queue
#include <unordered_map>      // unordered_map
#include <condition_variable> // condition_variable
#include "XTraits.hpp"        // function_traits
#include "XConnect.hpp"       // XConnect
#include "XObject.hpp"        // XConnectSavor

// How To Use Only
///
// std::cout << "main thread id " << std::this_thread::get_id() << std::endl;
// XApplication app;
// XSigalTest xt, xt2;
// xt.connect(
//     &xt, &XSigalTest::signal, &xt2, [](int a, int b, Data c)
//     { std::cout << "xt slot 1 thread id " << std::this_thread::get_id() << std::endl; },
//     XConnectType::QueuedConnection);
// xt.connect(
//     &xt, &XSigalTest::signal, &xt2, [](int a, int b, Data c)
//     { std::cout << "xt slot 2 thread id " << std::this_thread::get_id() << std::endl; },
//     XConnectType::QueuedConnection);
// xt2.connect(
//     &xt2, &XSigalTest::signal, &xt, [](int a, int b, Data c)
//     { std::cout << "xt2 slot 1 thread id " << std::this_thread::get_id() << std::endl; },
//     XConnectType::QueuedConnection);
// xt2.move_to_thread();
// std::this_thread::sleep_for(std::chrono::seconds(1));
// xt.signal(2, 2, Data{2, 3, "222222222"s}); // emit  a signal
// xt2.signal(2, 2, Data{2, 3, "222222222"s}); // emit  a signal
// auto starttime = XLocalTime::utcnow();
// return app.exec([&starttime]() -> bool
//                 { return XLocalTime::before<std::chrono::seconds>(starttime) < 3; });
namespace Novichu
{
    class XObject;
    class XApplication : public XSingleton<XApplication>, private XObject
    {
        friend class XEventLoop;
        friend class XObject;

    public:
        template <typename Fn, typename... Args>
        requires std::is_same_v<bool, typename function_traits<Fn>::return_type> && std::is_invocable_v<Fn, Args...>
        static int exec(Fn &&fn, Args &&...args);

    private:
    };

    template <typename Fn, typename... Args>
    requires std::is_same_v<bool, typename function_traits<Fn>::return_type> && std::is_invocable_v<Fn, Args...>
    int XApplication::exec(Fn &&fn, Args &&...args)
    {
        auto this_thread_id = std::this_thread::get_id();
        while (std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...))
        {
            std::unique_lock<std::mutex> lock(XConnectSavor::Instance().mutex());
            XConnectSavor::Instance().cv().wait_for(lock, std::chrono::seconds(1), [&this_thread_id]
                                                    { return XConnectSavor::Instance().getqueuesize() ||
                                                             XConnectSavor::Instance().exit(); });
            std::queue<Novichu::XConnectSavor::xc_args_type> queue;
            XConnectSavor::Instance().getqueue(this_thread_id).swap(queue);
            while (queue.size())
            {

                auto &&[xc, arg] = std::move(queue.front());
                if (auto c_obj = reinterpret_cast<XObject *>(xc.obj_ptr))
                    c_obj->invoke(xc, arg);
                queue.pop();
            }
        }
        XConnectSavor::Instance().set_exit();
        std::unique_lock lock(XConnectSavor::Instance().mutex());
        std::notify_all_at_thread_exit(XConnectSavor::Instance().cv(), std::move(lock));
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 0;
    }
}
#endif
