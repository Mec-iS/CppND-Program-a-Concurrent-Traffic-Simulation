#include <iostream>
#include <random>
#include <chrono>
#include <stdexcept>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mtx);

    if (!_queue.empty()) {
        _cond.wait(lck);
        auto msg = std::move(_queue.front());
        _queue.pop_front();
    
        return msg;
    } else {
        throw std::runtime_error("No element in the queue. Notification was incorrect.");
    }
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx);

    this->_queue.push_back(std::move(msg));

    _cond.notify_one();
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
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        _queue.receive();
        if(this->getCurrentPhase() == TrafficLightPhase::green) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public
    // method „simulate“ is called. To do this, use the thread queue in the base class.
    this->threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    int n = 0; // countdown
    while(true) {
        if (n == 0) {
            // toggle light signal
            this->togglePhase();
            auto phase = this->getCurrentPhase();
            _queue.send(std::move(phase));
            // reset the countdown to randint(4,6)
            n = rand() % 3 + 4;
        } else {
            // sleep 1 sec
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // countdown -1
            n -= 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

