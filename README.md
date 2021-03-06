# Logger
## Highlights
* Non-blocking network IO with Boost::ASIO
* Thread-safe logging message queue
* Highly modular, polymorphic request system
* Highly C++11 and STL "compliant"
* Smart Make system with auto dependency
* Persistent database with Boost::Serialization library (under construction)
* Unit tests done with Boost::Test library, coverage: everything except server-client communication.
## Platform
* Ubuntu 16.04 LTS
* GCC 5.4.0 (must support gcc-ar)
* GNU Make 4.0
* Boost Library 1.58 (Try this command on Ubuntu: `sudo apt-get install libboost-dev`)
* Python 2.7.12 (for running the client)
* Git (Of course)
## Build
* `git clone https://github.com/xza218/logger.git`
* `cd logger/server`
* `make -j` - this will build both server application and unit tests.
## Tutorial
* Start with pwd at logger root
* Start server first: `./server/build/bin/server` (the unit test is ./server/build/tests/server_test)
* Start client next: `./client/client.py`
* On client command line interface:
  * First try add a new log: `new_log JohnDoe info hello world I'm here!!!`, and press Enter to send it out, then press Enter AGAIN with an empty command so that the client reads the response from the server.
    * On the client window you'll receive server's response: `Ok`
  * Add another log at different priority level: `new_log BlackSmith error Panic!! I can't handle this I think I might as well just die.`. Press Enter twice like before.
    * You'll see the `Ok` response again on the client side.
      * Tips: Five log levels are: `debug`, `info`, `warning`, `critical` and `error`, from least severe to most severe. If you use anything other than those provided, you'll receive an error instead of `Ok`. Try it out!
  * Try to request a read from server. Enter `dump_all debug` then press Enter twice.
    * Aside from `Ok`, you'll receive the 2 messages you just sent
  * Try increase the debug level in the dump. Type `dump_all warning` and press Enter twice.
    * You'll see only the more severe message now!
  * Type `delete_all` and press Enter twice, then `dump_all` and press Enter twice, tell me what happened.
  * Try a wrong command name and press Enter twice. See what happened? 
## Unit Testing
* To build: Run `make -j` in `test` directory. If you've already run `make -j` in the `server` directory, you can skip this step.
* From `server` directory, run: `./build/tests/server_test  --log_level=message`
