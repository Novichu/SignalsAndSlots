#ifndef __XOBJECT__HPP__
#define __XOBJECT__HPP__

#include <unordered_set>     // unordered_set
#include <functional>        // functional std::invoke
#include "XTraits.hpp"       // function_traits
#include "XSerializer.hpp"   // XSerializer
#include "XConnect.hpp"      // XConnect
#include "XConnectSavor.hpp" // XConnect
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
    class XObject
    {
    public:
        X_XObject;

        template <typename Fn1, typename Fn2>
        requires std::derived_from<typename function_traits<Fn1>::object, XObject> && std::derived_from<typename function_traits<Fn2>::object, XObject> && is_args_count_same_v<Fn1, Fn2> && function_traits<Fn1>::value && function_traits<Fn2>::value
            XConnect
            connect(function_traits<Fn1>::object *obj1, Fn1 &&fn1, function_traits<Fn2>::object *obj2, Fn2 &&fn2, XConnectType contype = XConnectType::AutoConnection)
        {
            if (static_cast<XObject *>(obj1) != static_cast<XObject *>(this))
                return obj1->connect(obj1, std::forward<Fn1>(fn1), obj2, std::forward<Fn2>(fn2));
            std::unique_lock lock(_sp_mutex);
            XConnect xc{_get_obj_address(obj1), _get_function_value(fn1), _get_obj_address(obj2), 0, contype, std::this_thread::get_id()};
            auto &tid_xc = _ss_r_tid_xc[xc.obj1name + xc.fn1name][xc.obj2name];
            auto &slots = _ss_r_slots[xc.obj1name + xc.fn1name][xc.obj2name];
            xc.fn2id = tid_xc.size();
            _s_ss[xc.obj1name].emplace_back(xc.obj1name + xc.fn1name);
            slots.emplace_back([fn = std::forward<Fn2>(fn2), obj2, this](const std::string &in) mutable
                               { callproxy(std::forward<Fn2>(fn), obj2, in); });
            tid_xc.emplace_back(xc.tid, xc.contype);
            return xc;
        }
        template <typename Fn1, typename Obj2, typename Fn2>
        requires std::derived_from<typename function_traits<Fn1>::object, XObject> && std::derived_from<Obj2, XObject> && is_args_count_same_v<Fn1, Fn2> && function_traits<Fn1>::value &&(!function_traits<Fn2>::value)
            XConnect
            connect(function_traits<Fn1>::object *obj1, Fn1 &&fn1, Obj2 *obj2, Fn2 &&fn2, XConnectType contype = XConnectType::AutoConnection)
        {
            if (static_cast<XObject *>(obj1) != static_cast<XObject *>(this))
                return obj1->connect(obj1, std::forward<Fn1>(fn1), obj2, std::forward<Fn2>(fn2));
            std::unique_lock lock(_sp_mutex);
            XConnect xc{_get_obj_address(obj1), _get_function_value(fn1), _get_obj_address(obj2), 0, contype, std::this_thread::get_id()};
            auto &tid_xc = _ss_r_tid_xc[xc.obj1name + xc.fn1name][xc.obj2name];
            auto &slots = _ss_r_slots[xc.obj1name + xc.fn1name][xc.obj2name];
            xc.fn2id = tid_xc.size();
            _s_ss[xc.obj1name].emplace_back(xc.obj1name + xc.fn1name);
            slots.emplace_back([fn = std::forward<Fn2>(fn2), this](const std::string &in) mutable
                               { callproxy(std::forward<Fn2>(fn), in); });
            tid_xc.emplace_back(xc.tid, xc.contype);
            return xc;
        }
        template <typename Fn1>
        requires function_traits<Fn1>::value void
        disconnect(function_traits<Fn1>::object *obj1, Fn1 &&fn1)
        {
            std::unique_lock lock(_sp_mutex);
            _ss_r_slots[_get_obj_address(obj1) + _get_function_value(fn1)].clear();
        }
        void disconnect(XConnect &xc)
        {
            std::unique_lock lock(_sp_mutex);
            std::exchange(_ss_r_slots[xc.obj1name + xc.fn1name][xc.obj2name].at(xc.fn2id), nullptr);
        }
        template <typename Obj>
        requires std::derived_from<Obj, XObject>
        void disconnect(Obj *obj1)
        {
            std::unique_lock lock(_sp_mutex);
            for (auto obj_fn_anme : _s_ss[_get_obj_address(obj1)])
                _ss_r_slots[obj_fn_anme].clear();
        }
        template <typename Fn1, typename... Args>
        requires function_traits<Fn1>::value void active(function_traits<Fn1>::object *obj1, Fn1 &&fn1, Args &&...args)
        {
            XSerializer ds;
            package_params(ds, std::make_tuple(std::forward<Args>(args)...));
            uint64_t obj1name = _get_obj_address(obj1);
            uint64_t fn1name = _get_function_value(fn1);
            std::vector<XConnect> direct_calleds;
            {
                std::unique_lock lock(_sp_mutex);
                auto &tid_xc = _ss_r_tid_xc[obj1name + fn1name];
                auto &r_slots = _ss_r_slots[obj1name + fn1name];
                uint64_t slotsid = 0;
                for (auto [obj2name, slots] : r_slots)
                {
                    for (size_t slotsid = 0; slotsid < slots.size(); slotsid++)
                    {
                        XConnect xc{obj1name, fn1name, obj2name, slotsid, tid_xc[obj2name].at(slotsid).second, tid_xc[obj2name].at(slotsid).first};
                        if (!XConnectSavor::Instance().get_slot_tid(xc.obj2name) &&
                            ((xc.contype == XConnectType::AutoConnection && xc.tid == std::this_thread::get_id()) ||
                             xc.contype == XConnectType::DirectConnection))
                        {
                            direct_calleds.emplace_back(xc);
                        }
                        else
                        {
                            if (auto now_tid = XConnectSavor::Instance().get_slot_tid(xc.obj2name))
                                xc.tid = *now_tid;
                            xc.obj_ptr = this;
                            XConnectSavor::Instance().addactive_queue(xc, ds.data());
                        }
                    }
                }
            }
            for (auto &xc : direct_calleds)
            {
                invoke(xc, ds.data());
            }
        }
        void invoke(XConnect &xc, std::string &in)
        {
            std::unique_lock lock(_sp_mutex);
            if (auto &slot = _ss_r_slots[xc.obj1name + xc.fn1name][xc.obj2name].at(xc.fn2id))
            {
                try
                {
                    slot(in);
                }
                catch (...)
                {
                }
            }
        }
        void move_to_thread()
        {
            std::thread{
                [this]
                {
                    auto this_thread_id = std::this_thread::get_id();
                    XConnectSavor::Instance().force_change_tid(_get_obj_address(this), this_thread_id);
                    while (!XConnectSavor::Instance().exit())
                    {
                        std::unique_lock<std::mutex> lock(XConnectSavor::Instance().mutex());
                        XConnectSavor::Instance().cv().wait(lock, [&this_thread_id, this]
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
                    std::unique_lock lock(XConnectSavor::Instance().mutex());
                    std::notify_all_at_thread_exit(XConnectSavor::Instance().cv(), std::move(lock));
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }}
                .detach();
        }

    private:
        template <typename Fn>
        void callproxy(Fn &&fun, const std::string &in)
        {
            using fntrs = function_traits<Fn>;
            using args_type = fntrs::args_type;
            constexpr size_t args_count = fntrs::args_count;
            XSerializer ds{XBuffer(in)};
            args_type args = ds.get_tuple<args_type>(std::make_index_sequence<args_count>{});
            std::apply(std::forward<Fn>(fun), std::forward<args_type>(args));
        }
        template <typename Fn>
        requires std::is_class_v<Fn>
        void callproxy(Fn &&fun, const std::string &in)
        {
            using fntrs = function_traits<Fn>;
            using args_type = fntrs::args_type;
            constexpr size_t args_count = fntrs::args_count;
            XSerializer ds{XBuffer(in)};
            args_type args = ds.get_tuple<args_type>(std::make_index_sequence<args_count>{});
            std::apply(std::forward<Fn>(fun), std::forward<args_type>(args));
        }
        template <typename Fn>
        void callproxy(Fn &&fun, function_traits<Fn>::object *obj1, const std::string &in)
        {
            using fntrs = function_traits<Fn>;
            using args_type = fntrs::args_type;
            constexpr size_t args_count = fntrs::args_count;
            XSerializer ds{XBuffer(in)};
            args_type args = ds.get_tuple<args_type>(std::make_index_sequence<args_count>{});
            auto __apply_impl = []<size_t... Idx>(Fn && __f, function_traits<Fn>::object * obj, args_type && __t, std::index_sequence<Idx...>)
            {
                return std::invoke(std::forward<Fn>(__f), obj,
                                   std::get<Idx>(std::forward<args_type>(__t))...);
            };
            __apply_impl(std::forward<Fn>(fun), obj1, std::forward<args_type>(args), std::make_index_sequence<args_count>{});
        }

    private:
        using func_type = std::function<void(const std::string &)>;
        using tid_xct_pair = std::pair<std::thread::id, XConnectType>;
        spinlock_mutex _sp_mutex;                                                                           // for _ss_r_slots
        std::unordered_map<uint64_t, std::unordered_map<uint64_t, std::vector<func_type>>> _ss_r_slots;     // sender_siganl reciever slots
        std::unordered_map<uint64_t, std::unordered_map<uint64_t, std::vector<tid_xct_pair>>> _ss_r_tid_xc; // sender_siganl reciever tid_connect_type;
        std::unordered_map<uint64_t, std::vector<uint64_t>> _s_ss;                                          // sender siganls
    };
}
#endif
