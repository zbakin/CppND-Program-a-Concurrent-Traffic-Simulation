#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    std::unique_lock<std::mutex> uniqueLck(_mutex);
    _condition.wait(uniqueLck, [this] { return !_queue.empty(); });
    // to wait for and receive new messages and pull them from the queue using move semantics.
    T msg = std::move(_queue.back()); // back returns the last element's reference
    _queue.pop_back();
    // The received object should then be returned by the receive function.
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    std::lock_guard<std::mutex> lck(_mutex);
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    while(true) {
        _currentPhase = _messageQueue.receive();
    // Once it receives TrafficLightPhase::green, the method returns.
        if(_currentPhase == green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    auto prev = std::chrono::high_resolution_clock::now();
    auto curr = std::chrono::high_resolution_clock::now();
    while(true) {
        // wait 1ms between 2 cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
        curr = std::chrono::high_resolution_clock::now();
        // Calculate the duration between 2 cycles
        double duration = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev).count();
        std::random_device rand_device;
        std::uniform_int_distribution<int> distr(4000, 6000); // measureing in milliseconds
        double randomDuration = distr(rand_device);
        if(duration >= randomDuration) {
            _currentPhase = (_currentPhase == red) ? green : red;
            prev = std::chrono::high_resolution_clock::now();
        }
        _messageQueue.send(std::move(_currentPhase));
    }
}
