#include "EventManager.hpp"
#include <coroutine>

EventManager eventMgr;

nes::event_dispatcher &EventManager::getDispatcher() {
    return dispatcher;
}