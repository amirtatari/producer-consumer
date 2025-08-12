#include "tsqueue.hpp"

#include <iostream>
#include <chrono>

void ThreadSafeQueue::enqueue(int value)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_condition.wait(lock, [this] { return m_queue.size() < m_maxQueSize; });       // wait until queue has a space
    m_queue.push(value);
    m_condition.notify_one();
}

int ThreadSafeQueue::dequeue()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_condition.wait(lock, [this] { return m_queue.empty() == false; });    // wait until queue has at least one element
    // get the front element and remove it from queue
    int value = m_queue.front();
    m_queue.pop();
    m_condition.notify_all();
    return value;
}

void ThreadSafeQueue::producerMethod(std::stop_token stoken, int thread_idx)
{
    while(!stoken.stop_requested())
    {
        enqueue(thread_idx);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    std::cout << "ThreadSafeQueue::producerMethod " << thread_idx << " : thread stopped!\n";
}

void ThreadSafeQueue::consumerMethod(std::stop_token stoken)
{
    while(!stoken.stop_requested())
    {
        const int value = dequeue();
        std::cout << "ThreadSafeQueue::dequeue: recieved value=" << value << '\n';
    }
    std::cout << "ThreadSafeQueue::consumerMethod: thread stopped!\n";
}

void ThreadSafeQueue::run(size_t num_producers)
{
    // run producer threads
    m_producers.reserve(num_producers);
    for (size_t i = 0; i < num_producers; ++i)
    {
        m_producers.emplace_back([this, i](std::stop_token st) { producerMethod(st, i); });
    }

    // run consumer thread
    m_consumer = std::jthread([this](std::stop_token st) { consumerMethod(st); });
}

void ThreadSafeQueue::stop()
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