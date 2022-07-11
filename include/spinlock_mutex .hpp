
#ifndef __SPINLOCK_MUTEX__HPP__
#define __SPINLOCK_MUTEX__HPP__
#include <atomic> // std::atomic_flag
namespace Novichu
{
    class spinlock_mutex
    {
        std::atomic_flag flag = ATOMIC_FLAG_INIT;

    public:
        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire))
                ;
        }
        void unlock()
        {
            flag.clear(std::memory_order_release);
        }
    };
}
#endif