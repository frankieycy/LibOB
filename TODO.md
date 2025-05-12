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
    - takes in a `TimestampHandlerBase` ptr
    - order matching test cases
* `Market::OrderEventManager`
    - unified interface for order submmit, cancel, modify etc.
    - takes in a `TimestampHandlerBase` ptr
    - order id handler

### Analytics

### GUI

### Modeling
