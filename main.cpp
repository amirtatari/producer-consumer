#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <chrono>

class ThreadSafeQueue
{
    std::vector<std::jthread> m_producers;
    std::jthread m_consumer;
    std::queue<int> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_condition;
    const size_t m_maxQueSize = 10;

    void enqueue(int value)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_condition.wait(lock, [this] { return m_queue.size() < m_maxQueSize; });
        m_queue.push(value);
        m_condition.notify_one();
    }

    int dequeue()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_condition.wait(lock, [this] { return m_queue.empty() == false; });
        int value = m_queue.front();
        m_queue.pop();
        m_condition.notify_all();
        return value;
    }
    
    void producerMethod(std::stop_token stoken, int thread_idx)
    {
        while(!stoken.stop_requested())
        {
            enqueue(thread_idx);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        std::cout << "ThreadSafeQueue::producerMethod " << thread_idx << " : thread stopped!\n";
    }

    void consumerMethod(std::stop_token stoken)
    {
        while(!stoken.stop_requested())
        {
            int value = dequeue();
            std::cout << "ThreadSafeQueue::dequeue: recieved value=" << value << '\n';
        }
        std::cout << "ThreadSafeQueue::consumerMethod: thread stopped!\n";
    }

public:
    ~ThreadSafeQueue()
    {
        m_producers.clear();
        std::cout << "ThreadSafeQueue::ThreadSafeQueue: destructor called!\n";
    }
    
    void run()
    {
        m_producers.reserve(5);
        for (size_t i = 0; i < 5; ++i)
        {
            m_producers.emplace_back([this, i](std::stop_token st) { producerMethod(st, i); });
        }

        m_consumer = std::jthread([this](std::stop_token st) { consumerMethod(st); });
    }

    void stop()
    {
        // stop producer threads
        for (auto &producer : m_producers)
        {
            producer.request_stop();
        }

        // stop consumer thread
        m_consumer.request_stop();

        // wakeup all the sleeping threads
        m_condition.notify_all();

        std::cout << "ThreadSafeQueue::stop: all threads requested to stop!\n";
    }
};

int main()
{
    ThreadSafeQueue tsq;
    tsq.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
    tsq.stop();
    std::cout << "app finished!\n";
    return 0;
}