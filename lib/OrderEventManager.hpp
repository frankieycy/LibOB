#ifndef ORDER_EVENT_MANAGER_HPP
#define ORDER_EVENT_MANAGER_HPP
#include "Utils.hpp"
#include "OrderEvent.hpp"

namespace Market {
class OrderEventManagerBase {
public:
private:
    Utils::Counter::IdHandlerBase myOrderIdHandler;
    Utils::Counter::TimestampHandlerBase myOrderTimestampHandler;
};
}

#endif
