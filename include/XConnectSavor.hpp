#ifndef __XCONNECTSAVOR__HPP__
#define __XCONNECTSAVOR__HPP__
#include <unordered_map>       // unordered_map
#include <thread>              // std::thread::id
#include <optional>            // std::optional
#include "XSingleton.hpp"      // XSingleton
#include "spinlock_mutex .hpp" // spinlock_mutex

namespace Novichu
{
    class XConnectSavor : public XSingleton<XConnectSavor> // for reference
    {
        friend class XApplication;
        friend struct XConnect;
        friend class XEventLoop;
        friend class XObject;

    private:
        void force_change_tid(uint64_t obj2name, std::thread::id tid)
        {
            std::unique_lock lock(_sp_mutex2);
            _r_tids[obj2name] = tid;
        }
        std::optional<std::thread::id> get_slot_tid(uint64_t obj2name)
        {
            std::unique_lock lock(_sp_mutex2);
            if (_r_tids.count(obj2name))
                return _r_tids[obj2name];
            return std::nullopt;
        }
        void addactive_queue(XConnect xc, std::string &in);
        auto &getqueue(std::thread::id tid);
        auto getqueuesize();
        auto &mutex() { return _mutex; };
        auto &cv() { return _cv; };
        bool exit() { return !_running.load(); }
        void set_exit() { _running.store(false); }

    private:
        std::atomic<bool> _running = true;

        spinlock_mutex _sp_mutex2;                             // for _r_tids
        std::unordered_map<uint64_t, std::thread::id> _r_tids; // reciever thread_id

        using xc_args_type = std::pair<XConnect, std::string>;
        std::mutex _mutex;           //
        std::condition_variable _cv; // condition_variable_any
        std::unordered_map<std::thread::id, std::queue<xc_args_type>> _event_queue;
    };
    void XConnectSavor::addactive_queue(XConnect xc, std::string &in)
    {
        std::unique_lock lock(_mutex);
        _event_queue[xc.tid].emplace(xc, in);
        _cv.notify_all();
    }
    auto &XConnectSavor::getqueue(std::thread::id tid)
    {
        return _event_queue[tid];
    }
    auto XConnectSavor::getqueuesize()
    {
        return _event_queue.size();
    }
}
#endif