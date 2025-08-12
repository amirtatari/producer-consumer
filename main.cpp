
#include "tsqueue.hpp"

#include <iostream>  
#include <chrono>

int main()
{
    // run the threads
    ThreadSafeQueue tsq;
    tsq.run(5);

    // sleep for 20 seconds and then stop the threads 
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
    tsq.stop();

    return 0;
}