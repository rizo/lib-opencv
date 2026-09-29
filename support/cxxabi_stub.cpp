// Stub of __cxa_call_terminate for older versions of gcc.
// See:
//  - https://github.com/unikraft/lib-libcxx/issues/36
//  - https://stackoverflow.com/questions/79815074/linking-fails-with-in-function-main-cold-undefined-reference-to-cxa-call

#include <exception>

extern "C" void __cxa_call_terminate(void*) {
    std::terminate();
}
