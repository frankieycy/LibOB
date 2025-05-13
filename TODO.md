## TODO

### Infrastructure

* Misc. Utils
    - ability to cast Enum as string
* `Utils::Counter::TimestampHandlerBase`
    - world clock synchronized over `MatchingEngine` and `OrderEventManager`
    - unified timestamp generation function `tick()`
    - class for ticking unix timestamp
* `Exchange::MatchingEngine`
    - ok, display order book, trade log, market queue etc.
    - ok, trade id handler
    - ok, separate order book data structures from interface into base class
    - ok, take in a `TimestampHandlerBase` ptr
    - ok, move updated orders to the back of limit queue
    - ok, override incoming orders or order updates with the internal clock
    - ok, make `fillOrderByMatchingLimitQueue` etc. as class member function
    - order matching test cases
* `Market::OrderEventManager`
    - unified interface for order submmit, cancel, modify etc.
    - take in a `TimestampHandlerBase` ptr
    - order id handler

### Analytics

### GUI

### Modeling
