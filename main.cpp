#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

// This code prints FooBar n times, where "Foo" and "Bar" are printed by different threads.
// The synchronization is achieved with the use of an atomic variable, a condition variable and memory orders.

class Widget {
private:
    int n;
    std::condition_variable cv;
    std::mutex m;
    std::atomic_bool is_foo_turn;

public:
    Widget(int n) : n(n)
    {
        is_foo_turn.store(true, std::memory_order_release);
    }

    void foo (const std::function<void()>& printFoo)
    {
        for (int i{0}; i < n; i++) {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this]{ return is_foo_turn.load(std::memory_order_acquire); });
            printFoo();
            is_foo_turn.store(false, std::memory_order_release);
            cv.notify_one();
        }
    }
    
    void bar (const std::function<void()>& printBar)
    {
        for (int i{0}; i < n; i++) {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this]{ return !is_foo_turn.load(std::memory_order_acquire); });
            printBar();
            is_foo_turn.store(true, std::memory_order_release);
            cv.notify_one();
        }
    }
};

int main() {

    int n;
    std::cin >> n;

    Widget widget(n);

    std::thread f ([&widget]{
        widget.foo([]{
            std::cout<<"Foo";});
    });

    std::thread b ([&widget]{
        widget.bar([]{
            std::cout<<"Bar";});
    });

    f.join();
    b.join();
    return 0;
}
