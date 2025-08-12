#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

/* this class implements a thread-safe queue with multiple producer threads and a single consumer thread */
class ThreadSafeQueue
{
    std::vector<std::jthread> m_producers;          // vector of producer threads
    std::jthread m_consumer;                        // consumer thread
    std::queue<int> m_queue;                        // queue to hold the elements
    std::mutex m_mtx;                               // mutex to protect the queue
    std::condition_variable m_condition;            // condition variable to notify threads
    const size_t m_maxQueSize = 10;                 // maximum size of the queue

    // push the element into queue
    void enqueue(int value);

    // get and pop the front element from queue
    int dequeue();
    
    // producer method that runs in a separate thread
    void producerMethod(std::stop_token stoken, int thread_idx);

    // consumer mthod that also runs in a seprate thread
    void consumerMethod(std::stop_token stoken);

public:
    // method that starts producer threads with a single consumer thread
    void run(size_t num_producers);

    // method to stop all the running threads
    void stop();
};