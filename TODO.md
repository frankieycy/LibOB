## TODO

### Infrastructure

* Code Styling
    - ok, remove const on function value return
    - ok, return `shared_ptr` to const in class getters
    - ok, organize the lib hpp/cpp files into folders of their namespace e.g. `Utils` and `Market`
    - ok, avoid `return out` in `operator<<` if unneeded
    - ok, string util functions are to be put at file end after class definitions
    - ok, unify `to_string()` and `toString()`, deprecate the former
    - use EnumTraits idioms for all enum-to-string conversions
* Misc. Utils
    - ok, ability to cast Enum as string, `to_string`
    - ok, write up `Logger` class, use it for debug logging
    - ok, add `LogLevel` to logger to enable `logger << LogLevel::INFO << ...`
    - ok, break Utils.hpp into smaller specific util files when the set of functions becomes big enough
    - ok, output trace stack in `Error::LIB_THROW` for debugging
* `RegressionTests`
    - ok, cpp files in `Inputs` (small selections from `Tests`) and txt output files in `Baseline` (outputs from running cpp)
    - ok, add `FILTER` makefile flag to run specific tests
    - add `RegressionTests` and `UpdateBaselines` to makefile
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
    - ok, order matching test cases - submit, matching, cancel, modify, callback etc.
    - ok, order book data members size reserve to reduce memory reallocation in STL containers
    - ok, implement constructors for `MatchingEngine` and `OrderEventManager`
    - ok, deprecate single callback function (e.g. myOrderProcessingCallback) and use callbacks vector instead
    - ASCII order book dynamic display (time evolution of book)
    - when will `OrderProcessingReport` report failure?
    - order book construction from events stream, same for `OrderEventManager`
    - detailed speed profiling for various matching engine operations
* `Exchange::ITCHMessage`
    - ok, inheritance hierachy of ITCH messages, each as a minimal and compact representation of `OrderProcessingReport`
    - ok, implement `encodeReport` for each class of `OrderProcessingReport`
    - ok, add `ITCHMessage` reg test
    - ok, implement `makeEvent` hence `MatchingEngine::build`
* `Market::OrderEventBase`
    - ok, implement `OrderCancelAndReplaceEvent` compliant with ITCH protocol (alternative to `OrderModifyEvent`)
* `Market::OrderEventManager`
    - ok, unified interface for order submmit, cancel, modify etc.
    - ok, take in a `TimestampHandlerBase` ptr
    - ok, order id handler
    - ok, sync with `MatchingEngine` via a callback function on `OrderExecutionReport`
    - ok, add order cancel-replace reg test
* `Analytics::MatchingEngineMonitor`
    - ok, order book stats snapshot in a more human-readable format
    - ok, (bug) premature fetching of top levels snapshot before the executed queue is eliminated from book (e.g. market buy eats out entire top ask)
    - ok, (bug) `OrderBookTopLevelsSnapshot` iterators point to the order book subject to modifications later on, need to deep-copy the levels
    - ok, export order book time evolution as lobster csv format using `TimeSeriesCollector` members
    - ok, order event processing latency measurement
* `Analytics::MonitorOutputsAnalyzer`
    - ok, complete some basic book analytics structs, like spreads, depths, price returns etc.
    - ok, implement `runAnalytics()` function to populate the analytics structs from monitor outputs
* `Parser::LobsterDataParser`
* `Simulator::ZeroIntelligence`
    - ok, implement the `advance` and `simulate` functions in `ExchangeSimulatorBase`
    - ok, implement the next-event generator in `ZeroIntelligenceExchangeSimulator`
    - ok, test the behaviors of the engine, order manager and monitor for small books
    - ok, Santa Fe simulation experiments, appropriate choice of price/size/cancel sampling functors
    - study book asymptotics under the simplest assumption of sampling functors

### Analytics

### GUI

### Modeling

### References
