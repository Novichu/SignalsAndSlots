#include <iostream>
#include <chrono>
#include "./include/XApplication.hpp"
using namespace Novichu;
using namespace std::literals;

class XSigalTest : public XObject
{
public:
    void signal(int a, int b, int c) XMakeSignal(XSigalTest::signal, a, b, c); // signal
    void slot(int a, int b, int c)                                             // slot1
    {
        std::cout << a << b << c << std::endl;
        std::cout << " void XSigalTest::slot(int a, int b, int c) " << std::endl;
        std::cout << "thread id " << std::this_thread::get_id() << std::endl;
    }
};
void slot(int a, int b, int c) // slot2
{
    std::cout << a << b << c << std::endl;
    std::cout << " void slot(int a, int b, int c) " << std::endl;
    std::cout << "thread id " << std::this_thread::get_id() << std::endl;
}
int main(int argc, char **argv)
{
    XApplication app;
    XSigalTest xt, xt2;
    xt.connect(&xt, &XSigalTest::signal, &xt2, &XSigalTest::slot);
    xt.connect(&xt, &XSigalTest::signal, &xt2, &slot);
    xt.connect(
        &xt, &XSigalTest::signal, &xt2, [](int a, int b, int c)
        {
            std::cout << "a " << a << std::endl;
            std::cout << "b " << b << std::endl;
            std::cout << "c " << c << std::endl;
            std::cout << " lambda (int a, int b, int c) " << std::endl;
            std::cout << "thread id " << std::this_thread::get_id() << std::endl; },
        XConnectType::QueuedConnection);
    xt2.move_to_thread();
    std::cout << "main thread id " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    X_Emit xt.signal(2, 2, 3); // emit  a signal

    auto starttime = std::chrono::high_resolution_clock::now();
    return app.exec([&starttime]() -> bool
                    { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - starttime).count() > 3; });
    return 0;
}