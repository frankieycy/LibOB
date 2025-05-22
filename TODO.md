## TODO

### Infrastructure

* Code Styling
    - ok, remove const on function value return
    - ok, return `shared_ptr` to const in class getters
    - ok, organize the lib hpp/cpp files into folders of their namespace e.g. `Utils` and `Market`
* Misc. Utils
    - ok, ability to cast Enum as string, `to_string`
    - ok, write up `Logger` class, use it for debug logging
    - ok, add `LogLevel` to logger to enable `logger << LogLevel::INFO << ...`
    - break Utils.hpp into smaller specific util files when the set of functions becomes big enough
* `Utils::Counter::TimestampHandlerBase`
    - ok, world clock synchronized over `MatchingEngine` and `OrderEventManager`
    - ok, unified timestamp generation function `tick()`
    - class for ticking unix timestamp
* `Exchange::MatchingEngine`
    - ok, display order book, trade log, market queue etc.
    - ok, trade id handler
    - ok, separate order book data structures from interface into base class
    - ok, take in a `TimestampHandlerBase` ptr
    - ok, move updated orders to the back of limit queue
    - ok, override incoming orders or order updates with the internal clock
    - ok, make `fillOrderByMatchingLimitQueue` etc. as class member function
    - order matching test cases - submit, matching, cancel, modify, callback etc.
    - ASCII order book dynamic display (time evolution of book)
    - when will `OrderProcessingReport` report failure?
    - order book construction from events stream, same for `OrderEventManager`
* `Market::OrderEventManager`
    - ok, unified interface for order submmit, cancel, modify etc.
    - ok, take in a `TimestampHandlerBase` ptr
    - ok, order id handler
    - ok, sync with `MatchingEngine` via a callback function on `OrderExecutionReport`

### Analytics

### GUI

### Modeling
