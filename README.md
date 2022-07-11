# SignalsAndSlots 信号槽
## 介绍
用于简单实现Qt中的信号槽机制
1. 不依赖于Qt而方便的使用信号槽机制
2. 信号和槽是多对多的关系。一个信号可以连接多个槽，而一个槽也可以监听多个信号。
3. 仅支持Qt5以上的信号槽连接机制
4. 使用到了序列化和反序列化
### 使用前必看
- 语言标准 C++ 20 (使用到了concept)
- 槽函数支持所有可调用对象
- 支持 move_to_thread ,将指定对象的所有参数在新线程执行
- 其他使用基本同于Qt
### 如何使用
#### 1. 如下文定义
>       class XSigalTest : public XObject // 继承自 XObject 即可 无需使用XObject 宏
>       {
>       public:
>           void signal(int a, int b, int c) XMakeSignal(XSigalTest::signal, a, b, c); // signal // 使用宏定义信号
>            void slot(int a, int b, int c)                                             // slot1
>           {   
>               std::cout << a << b << c << std::endl;
>               std::cout << " void XSigalTest::slot(int a, int b, int c) " << std::endl;
>               std::cout << "thread id " << std::this_thread::get_id() << std::endl;
>           }   
>       };   
>       void slot(int a, int b, int c) // slot2
>       {   
>           std::cout << a << b << c << std::endl;
>           std::cout << " void slot(int a, int b, int c) " << std::endl;
>           std::cout << "thread id " << std::this_thread::get_id() << std::endl;
>       }

#### 2. 定义信号
在类内定义一个函数 并使用宏XMakeSignal按照如下输入即可
> void signal(int a, int b, int c) XMakeSignal(XSigalTest::signal, a, b, c); 
#### 3.使用案例 

>       int main(int argc, char **argv)
>         {
>             XApplication app;
>             XSigalTest xt, xt2;
>             xt.connect(&xt, &XSigalTest::signal, &xt2, &XSigalTest::slot);
>             xt.connect(&xt, &XSigalTest::signal, &xt2, &slot);
>             xt.connect(
>                &xt, &XSigalTest::signal, &xt2, [](int a, int b, int c)
>                {
>                    std::cout << "a " << a << std::endl;
>                    std::cout << "b " << b << std::endl;
>                    std::cout << "c " << c << std::endl;
>                    std::cout << " lambda (int a, int b, int c) " << std::endl;
>                    std::cout << "thread id " << std::this_thread::get_id() << std::endl; },
>                XConnectType::QueuedConnection);
>            xt2.move_to_thread();
 >           std::cout << "main thread id " << std::this_thread::get_id() << std::endl;
>            std::this_thread::sleep_for(std::chrono::seconds(1));
>            X_Emit xt.signal(2, 2, 3); // emit  a signal
>
>            auto starttime = std::chrono::high_resolution_clock::now();
>            return app.exec([&starttime]() -> bool
>                            { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() -   starttime).count() < 3; });
>            return 0;
>        }

#### 4. 输出效果
>        main thread id 16388
>        223
>        void XSigalTest::slot(int a, int b, int c)
>        thread id 16500
>        223
>        void slot(int a, int b, int c)
>        thread id 16500
>        a 2
>        b 2
>        c 3
>        lambda (int a, int b, int c)
>        thread id 16500
>
>
### 使用自定义变量
#### 作者提供了序列化的方法来实现对于用户自定义类型的扩展
>        struct Data
>        {
>            Data() {}
>            int b = 0;
>            int c = 0;
>            std::string s;
>        };
>       XSerializerRegisterSType(Data, b, c, s); // 使用 XSerializerRegisterSType 来注册这个类型 否者无法在序列化使用
序列化方法有两种 其中
1. 使用宏注册
>        XSerializerRegisterSType(Data, b, c, s);

2. 模板特化
>         template <>
>        inline void XSerializer::output_type<Data>(Data &t)
>        {
>            output_type(t.b);
>            output_type(t.c);
>            output_type(t.s);
>        }
>        template <>
>        inline void XSerializer::input_type<Data>(Data &t)
>        {
>            input_type(t.b);
>            input_type(t.c);
>            input_type(t.s);
>        }
3. 自定义变量使用示例
>       int main(int argc, char **argv)
>       {
>               XApplication app;
>               XSigalTest xt, xt2;
>               xt.connect(
>                   &xt, &XSigalTest::signal, &xt2, [](int a, int b, Data c)
>                   {
>                       std::cout << "a " << a << std::endl;
>                       std::cout << "b " << b << std::endl;
>                       std::cout << "c.b " << c.b << std::endl;
>                       std::cout << "c.c " << c.c << std::endl;
>                       std::cout << "c.s.size()  " << c.s.size()  <<"  c.s " << c.s << std::endl;
>                       std::cout << "thread id " << std::this_thread::get_id() << std::endl; },
>                   XConnectType::QueuedConnection);
>               xt2.move_to_thread();
>               std::cout << "main thread id " << std::this_thread::get_id() << std::endl;
>               std::this_thread::sleep_for(std::chrono::seconds(1));
>               xt.signal(2, 2, Data{2, 3, "123456789"s}); // emit  a signal
>               auto starttime = std::chrono::high_resolution_clock::now();
>               return app.exec([&starttime]() -> bool
>                            { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() -   starttime).count() < 3; });
>       }